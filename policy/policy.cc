#include "policy.h"

#include "lexer.h"
#include "synan.h"
#include "ast.h"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <fstream>
#include <list>


static std::map<std::string, std::shared_ptr<topology_t>> get_simple_topologies(const std::shared_ptr<ast_node_t>& ast);
static std::map<std::string, std::shared_ptr<topology_t>>& add_expr_topologies(
		const std::shared_ptr<ast_node_t>& ast,
		std::map<std::string, std::shared_ptr<topology_t>>& topologies);
static std::shared_ptr<topology_basic_t> construct_expr_topology(
		std::shared_ptr<ast_expr_t>& expr,
		std::map<std::string, std::shared_ptr<topology_t>>& topologies,
		std::shared_ptr<topology_basic_t>& arg);

static std::vector<pg_t> get_pgs(const std::shared_ptr<ast_node_t>& ast,
	const topology_basic_t& topology);

static inline std::string remove_space(const std::string& s);

static void topological_sort_dfs(
		const std::vector<std::vector<uint8_t>>& m,
		const int index,
		std::vector<bool>& discovered,
		std::vector<int>& end_time,
		int& time,
		std::list<int>& topological_order);
static std::list<int> topological_ordering(const std::vector<std::vector<uint8_t>>& m);


policy_t::policy_t(const char *file_path) {
	std::vector<symbol_t> symbols = lexify(file_path);

	dertree_t tree = parse_source(symbols);
	std::shared_ptr<ast_node_t> ast = ast_construct(tree, nullptr);

	topologies = get_simple_topologies(ast);
	topologies = add_expr_topologies(ast, topologies);

	topology = std::make_shared<topology_basic_t>("Total");

	for (auto& tuple : topologies) {
		if (auto t = std::dynamic_pointer_cast<topology_basic_t>(tuple.second)) {
			for (auto& m : t->index_mapping()) {
				tags.insert(m.first);
			}
			topology->disjoint_union(topology, t);
		}
		if (auto t = std::dynamic_pointer_cast<topology_linear_t>(tuple.second)) {
			for (auto &s : t->get_tags()) {
				tags.insert(s);
			}
			auto converted = std::make_shared<topology_basic_t>(*t);
			topology->disjoint_union(topology, converted);
		}
	}

	tags.insert("unknown");
	topology->add_unknown();

	// check DAG
	topological_ordering(topology->matrix());

	perimeter_guards = get_pgs(ast, *topology);
}

static std::map<std::string, std::shared_ptr<topology_t>> get_simple_topologies(const std::shared_ptr<ast_node_t>& ast) {
	std::map<std::string, std::shared_ptr<topology_t>> topologies;
	if (auto source = std::dynamic_pointer_cast<ast_source_t>(ast)) {
		for (auto& decl : source->get_decls()) {
			if (auto t = std::dynamic_pointer_cast<ast_topology_basic_t>(decl)) {
				if (topologies.find(t->get_name()) != topologies.end()) {
					std::ostringstream oss;
					oss << "Topology '" << t->get_name() << "' cannot be declared twice!";
					throw std::runtime_error(oss.str());
				}
				std::set<std::string> vertices;
				for (auto& edge : t->get_edges()) {
					vertices.insert(edge->get_source()->get_name());
					vertices.insert(edge->get_end()->get_name());
				}
				auto basic = std::make_shared<topology_basic_t>(t->get_name(), vertices);
				for (auto& edge : t->get_edges()) {
					basic->add_edge(edge->get_source()->get_name(), edge->get_end()->get_name());
				}

				topologies[t->get_name()] = basic;
			} else if(auto t = std::dynamic_pointer_cast<ast_topology_linear_t>(decl)) {
				if (topologies.find(t->get_name()) != topologies.end()) {
					std::ostringstream oss;
					oss << "Topology '" << t->get_name() << "' cannot be declared twice!";
					throw std::runtime_error(oss.str());
				}
				auto linear = std::make_shared<topology_linear_t>(t->get_name());
				for (auto& tag : t->get_tags()) {
					linear->add_tag(tag->get_name());
				}
				topologies[t->get_name()] = linear;
			}
		}
	}
	return topologies;
}

static std::map<std::string, std::shared_ptr<topology_t>>& add_expr_topologies(
		const std::shared_ptr<ast_node_t>& ast,
		std::map<std::string, std::shared_ptr<topology_t>>& topologies) {
	if (auto source = std::dynamic_pointer_cast<ast_source_t>(ast)) {
		for (auto& decl : source->get_decls()) {
			if (auto t = std::dynamic_pointer_cast<ast_topology_expr_t>(decl)) {
				if (topologies.find(t->get_name()) != topologies.end()) {
					std::ostringstream oss;
					oss << "Topology '" << t->get_name() << "' cannot be declared twice!";
					throw std::runtime_error(oss.str());
				}
				auto topology = std::make_shared<topology_basic_t>(t->get_name());
				topology = construct_expr_topology(t->get_expr(), topologies, topology);
				topology->set_name_prefix(t->get_name());
				topologies[t->get_name()] = topology;
			}
		}
	}
	return topologies;
}

static std::shared_ptr<topology_basic_t> construct_expr_topology(
		std::shared_ptr<ast_expr_t>& expr,
		std::map<std::string, std::shared_ptr<topology_t>>& topologies,
		std::shared_ptr<topology_basic_t>& arg) {
	if (auto e = std::dynamic_pointer_cast<ast_expr_bin_t>(expr)) {
		auto lhs = construct_expr_topology(e->get_lhs(), topologies, arg);
		auto rhs = construct_expr_topology(e->get_rhs(), topologies, arg);
		switch (e->get_oper()) {
			case ast_expr_bin_t::Oper::SUM: {
				arg->disjoint_union(lhs, rhs);
				return arg;
			}
			case ast_expr_bin_t::Oper::MUL: {
				arg->carthesian_product(lhs, rhs);
				return arg;
			}
			default:
				throw std::runtime_error("Unsupported binary operaion!");
		}
	} else if (auto e = std::dynamic_pointer_cast<ast_tag_t>(expr)) {
		try {
			std::shared_ptr<topology_t>& t = topologies.at(e->get_name());
			if (auto tl = std::dynamic_pointer_cast<topology_linear_t>(t)) {
				return std::make_shared<topology_basic_t>(*tl);
			} else if (auto tb = std::dynamic_pointer_cast<topology_basic_t>(t)) {
				return tb;
			}
			std::ostringstream oss;
			oss << "Topology '" << e->get_name() << "' should be linear or basic!";
			throw std::runtime_error(oss.str());
		} catch (std::out_of_range& err) {
			std::ostringstream oss;
			oss << "Unknown topology: '" << e->get_name() << "'!";
			throw std::runtime_error(oss.str());
		}
	} else {
		throw std::runtime_error("Unknown expression!");
	}
}

static std::vector<pg_t> get_pgs(const std::shared_ptr<ast_node_t>& ast,
		const topology_basic_t& topology) {
	std::vector<pg_t> perimeter_guards;
	if (auto source = std::dynamic_pointer_cast<ast_source_t>(ast)) {
		for (auto& decl : source->get_decls()) {
			if (auto t = std::dynamic_pointer_cast<ast_pg_t>(decl)) {
				try {
					auto tag = topology.get_index(t->get_tag());
					pg_t pg(t->get_name(), t->get_file(), tag);
					perimeter_guards.push_back(pg);
				} catch (std::out_of_range& e) {
					std::ostringstream oss;
					oss << "Unknown tag for perimeter guard '" << t->get_name()
						<< "': '"<< t->get_tag() << "'!";
					throw std::runtime_error(oss.str());
				}
			}
		}
	}
	return perimeter_guards;
}

topology_basic_t::topology_basic_t(const std::string& n) {
	name = n;
	mvertices = std::vector<std::vector<uint8_t>>();
	toindex = std::map<std::string, int>();
	fromindex = std::map<int, std::string>();
}

topology_basic_t::topology_basic_t(
		const std::string& n,
		const std::set<std::string>& vertices) {
	name = n;
	mvertices = std::vector<std::vector<uint8_t>>(
		vertices.size(), std::vector<uint8_t>(vertices.size()));
	toindex = std::map<std::string, int>();
	fromindex = std::map<int, std::string>();
	int i = 0;
	for (auto& v : vertices) {
		toindex[fullname(v)] = i;
		fromindex[i] = fullname(v);
		mvertices[i][i] = 1;
		i++;
	}
}

topology_basic_t::topology_basic_t(topology_linear_t& t) {
	name = t.get_name();
	size_t n = t.get_tags().size();
	mvertices = std::vector<std::vector<uint8_t>>(n, std::vector<uint8_t>(n));
	for (size_t i = 0; i < n; i++) {
		mvertices[i][i] = 1;
		if (i + 1 < mvertices.size()) {
			mvertices[i][i + 1] = 1;
		}
		std::string tag = remove_space(t.get_tags().at(i));
		toindex[tag] = i;
		fromindex[i] = tag;
	}
}

void topology_basic_t::add_edge(
		const std::string& source,
		const std::string& end) {
	int i = toindex.at(fullname(source));
	int j = toindex.at(fullname(end));
	mvertices[i][j] = 1;
}

void topology_basic_t::print() {
	std::cout << "Topology: '" << name << "'" << std::endl;
	for (auto& kv : toindex) {
		std::cout << "\t'" << kv.first << "', " <<  kv.second << ":";
		for (size_t j = 0; j < mvertices.size(); j++) {
			std::cout << " " << (int) mvertices[kv.second][j];
		}
		std::cout << std::endl;
	}
}

void topology_basic_t::carthesian_product(
		const std::shared_ptr<topology_basic_t>& t1,
		const std::shared_ptr<topology_basic_t>& t2) {
	size_t n = t1->size();
	size_t m = t2->size();
	auto& a = t1->matrix();
	auto& b = t2->matrix();
	auto& mapping_a = t1->index_mapping();
	auto& mapping_b = t2->index_mapping();

	std::map<std::string, int> r_toindex;
	std::map<int, std::string> r_fromindex;
	for (auto& tuple_a : mapping_a) {
		for (auto& tuple_b : mapping_b) {
			std::string name = "(" +  tuple_a.first + "," + tuple_b.first + ")";
			int index = tuple_a.second * m + tuple_b.second;
			std::string tag = remove_space(name);
			r_toindex[tag] = index;
			r_fromindex[index] = tag;
		}
	}
	toindex.clear();
	toindex = r_toindex;
	fromindex.clear();
	fromindex = r_fromindex;

	std::vector<std::vector<uint8_t>> r(n * m, std::vector<uint8_t>(n * m));

	/* Do R = A (x) I_2 */
	for (size_t i = 0; i < n; i++) {
		for (size_t j = 0; j < n; j++) {
			for (size_t ri = 0; ri < m; ri++) {
				r[i * m + ri][j * m + ri] = a[i][j];
			}
		}
	}

	/* Do R += I_1 (x) B */
	for (size_t ri = 0; ri < m * n; ri += m) {
		for (size_t i = 0; i < m; i++) {
			for (size_t j = 0; j < m; j++) {
				r[ri + i][ri + j] |= b[i][j];
			}
		}
	}

	for (size_t i = 0; i < mvertices.size(); i++) {
		mvertices[i].clear();
	}
	mvertices.clear();
	mvertices = r;
}

void topology_basic_t::disjoint_union(
		const std::shared_ptr<topology_basic_t>& t1,
		const std::shared_ptr<topology_basic_t>& t2) {
	size_t n = t1->size();
	size_t m = t2->size();
	auto& a = t1->matrix();
	auto& b = t2->matrix();
	auto& mapping_a = t1->index_mapping();
	auto& mapping_b = t2->index_mapping();

	std::map<std::string, int> r_toindex;
	std::map<int, std::string> r_fromindex;
	for (auto& tuple : mapping_a) {
		std::string tag = remove_space(tuple.first);
		r_toindex[tag] = tuple.second;
		r_fromindex[tuple.second] = tag;
	}
	for (auto& tuple : mapping_b) {
		std::string tag = remove_space(tuple.first);
		r_toindex[tag] = n + tuple.second;
		r_fromindex[n + tuple.second] = tag;
	}
	toindex.clear();
	toindex = r_toindex;
	fromindex.clear();
	fromindex = r_fromindex;

	std::vector<std::vector<uint8_t>> r(n + m, std::vector<uint8_t>(n + m));

	/* Perform direct sum of matrices */
	for (size_t i = 0; i < n; i++) {
		for (size_t j = 0; j < n; j++) {
			r[i][j] = a[i][j];
		}
	}

	for (size_t i = 0; i < m; i++) {
		for (size_t j = 0; j < m; j++) {
			r[n + i][n + j] = b[i][j];
		}
	}

	for (size_t i = 0; i < mvertices.size(); i++) {
		mvertices[i].clear();
	}
	mvertices.clear();
	mvertices = r;
}

void topology_basic_t::set_name_prefix(const std::string& prefix) {
	std::map<std::string, int> updated_to;
	std::map<int, std::string> updated_from;
	for (auto& t : toindex) {
		std::string r = remove_space(prefix + "." + t.first);
		updated_to[r] = t.second;
		updated_from[t.second] = r;
	}
	toindex.clear();
	toindex = updated_to;
	fromindex.clear();
	fromindex = updated_from;
}

std::string topology_t::fullname(const std::string& tag) {
	return remove_space(name + "." + tag);
}

bool policy_t::contains_tag(const std::string& tag) const {
	std::string cleaned = remove_space(tag);
	return tags.find(cleaned) != tags.end();
}

static inline std::string remove_space(const std::string& s) {
	std::string r = s;
	r.erase(std::remove_if(r.begin(), r.end(), isspace), r.end());
	return r;
}

int policy_t::tag_index(const std::string& tag) const {
	return topology->get_index(tag);
}

int topology_basic_t::get_index(const std::string& tag) const {
	return toindex.at(remove_space(tag));
}

std::string topology_basic_t::get_tag(int index) const {
	return fromindex.at(index);
}

int topology_linear_t::get_index(const std::string& tag) const {
	std::string cleaned = remove_space(tag);
	for (size_t i = 0; i < tags.size(); i++) {
		if (tags[i] == cleaned) {
			return i;
		}
	}
	std::ostringstream oss;
	oss << "Tag '" << tag << "' not in the topology!";
	throw std::runtime_error(oss.str());
}

void topology_basic_t::add_unknown() {
	auto new_toindex = std::map<std::string, int>();
	auto new_fromindex = std::map<int, std::string>();
	for (auto& t : toindex) {
		new_toindex[t.first] = t.second + 1;
		new_fromindex[t.second + 1] = t.first;
	}
	new_toindex["unknown"] = 0;
	new_fromindex[0] = "unknown";
	toindex.clear();
	toindex = new_toindex;
	fromindex.clear();
	fromindex = new_fromindex;

	for (auto& row : mvertices) {
		row.emplace(row.begin(), 0);
	}
	auto unknowns = std::vector<uint8_t>(mvertices.size() + 1, 1);
	mvertices.emplace(mvertices.begin(), unknowns);
}

void policy_t::dump(std::ofstream& out) {
	out << topology->size() << " " << perimeter_guards.size() << std::endl;
	for (size_t i = 0; i < lca_matrix.size(); i++) {
		out << topology->get_tag(i);
		for (size_t j = 0; j < lca_matrix[i].size(); j++) {
			out << " " << (int) lca_matrix[i][j];
		}
		out << std::endl;
	}

	for (auto& pg : perimeter_guards) {
		out << pg.name << " \"" << pg.file << "\" " << (int) pg.tag << std::endl;
	}
}



static void topological_sort_dfs(
		const std::vector<std::vector<uint8_t>>& m,
		const int index,
		std::vector<bool>& discovered,
		std::vector<int>& end_time,
		int& time,
		std::list<int>& topological_order) {
	discovered[index] = true;
	for (size_t j = 0; j < m[index].size(); j++) {
		if (m[index][j] > 0 && !discovered[j]) {
			topological_sort_dfs(m, j, discovered, end_time, time, topological_order);
		}
	}
	end_time[index] = time;
	topological_order.push_front(index);
	time++;
}

static std::list<int> topological_ordering(const std::vector<std::vector<uint8_t>>& m) {
	std::vector<bool> discovered(m.size());
	std::vector<int> end_time(m.size());
	int time = 0;
	std::list<int> topological_order;

	for (size_t i = 0; i < m.size(); i++) {
		if (!discovered[i]) {
			topological_sort_dfs(m, i, discovered, end_time, time, topological_order);
		}
	}

	for (size_t i = 0; i < m.size(); i++) {
		for (size_t j = 0; j < m.size(); j++) {
			if (i != j && m[i][j] > 0 && end_time[i] <= end_time[j]) {
				std::ostringstream oss;
				oss << "The policy is not a directed acyclical graph!";// <<
				throw std::runtime_error(oss.str());
			}
		}
	}
	return topological_order;
}