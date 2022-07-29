#include "policy.h"

#include "lexer.h"
#include "synan.h"
#include "ast.h"

#include <iostream>
#include <sstream>
#include <stdexcept>


static std::map<std::string, std::shared_ptr<topology_t>> get_simple_topologies(const std::shared_ptr<ast_node_t>& ast);
static std::map<std::string, std::shared_ptr<topology_t>>& add_expr_topologies(
		const std::shared_ptr<ast_node_t>& ast,
		std::map<std::string, std::shared_ptr<topology_t>>& topologies);
static std::shared_ptr<topology_basic_t> construct_expr_topology(
		std::shared_ptr<ast_expr_t>& expr,
		std::map<std::string, std::shared_ptr<topology_t>>& topologies,
		std::shared_ptr<topology_basic_t>& arg);


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

topology_basic_t::topology_basic_t(const std::string& n) {
	name = n;
	mvertices = std::vector<std::vector<uint8_t>>();
	toindex = std::map<std::string, int>();
}

topology_basic_t::topology_basic_t(
		const std::string& n,
		const std::set<std::string>& vertices) {
	name = n;
	mvertices = std::vector<std::vector<uint8_t>>();
	toindex = std::map<std::string, int>();
	int i = 0;
	for (auto& v : vertices) {
		toindex[fullname(v)] = i;
		mvertices.push_back(std::vector<uint8_t>(vertices.size()));
		mvertices[i][i] = 1;
		i++;
	}
}

topology_basic_t::topology_basic_t(topology_linear_t& t) {
	name = t.get_name();
	size_t n = t.get_tags().size();
	mvertices = std::vector<std::vector<uint8_t>>(n);
	for (size_t i = 0; i < n; i++) {
		mvertices[i] = std::vector<uint8_t>(n);
		mvertices[i][i] = 1;
		if (i + 1 < mvertices.size()) {
			mvertices[i][i + 1] = 1;
		}
		toindex[t.get_tags().at(i)] = i;
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

	std::map<std::string, int> rindex;
	for (auto& tuple_a : mapping_a) {
		for (auto& tuple_b : mapping_b) {
			std::string name = "(" +  tuple_a.first + " * " + tuple_b.first + ")";
			rindex[name] = tuple_a.second * m + tuple_b.second;
		}
	}
	toindex.clear();
	toindex = rindex;

	std::vector<std::vector<uint8_t>> r(n * m);
	for (size_t i = 0; i < r.size(); i++) {
		r[i] = std::vector<uint8_t>(n * m);
	}

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

	std::map<std::string, int> rindex;
	for (auto& tuple : mapping_a) {
		rindex[tuple.first] = tuple.second;
	}
	for (auto& tuple : mapping_b) {
		rindex[tuple.first] = n + tuple.second;
	}
	toindex.clear();
	toindex = rindex;

	std::vector<std::vector<uint8_t>> r(n + m);
	for (size_t i = 0; i < r.size(); i++) {
		r[i] = std::vector<uint8_t>(r.size());
	}

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
	std::map<std::string, int> updated;
	for (auto& t : toindex) {
		updated[prefix + "." + t.first] = t.second;
	}
	toindex.clear();
	toindex = updated;
}