#ifndef _POLICY_AST_H_
#define _POLICY_AST_H_

#include <vector>
#include <string>
#include <iostream>
#include <memory>

#include "synan.h"


/* Interfaces */

/* Basic AST node */
class ast_node_t {
	public:
		virtual ~ast_node_t() = 0;
		virtual void print() = 0;
};

/* Represents an expression - used for tags and expression topologies */
class ast_expr_t : public ast_node_t {
	public:
		virtual ~ast_expr_t() = 0;
		virtual void print() {}
};

/* Represents a declaration */
class ast_decl_t : public ast_node_t {
	public:
		virtual ~ast_decl_t() = 0;
		virtual void print() {}
};

/* Topology base class */
class ast_topology_t : public ast_decl_t {
	public:
		virtual ~ast_topology_t() {}
		void set_name(const std::string& n) {
			name = n;
		}
		std::string& get_name() {
			return name;
		}
		virtual void print() {}
	protected:
		std::string name;
};

/* Derived classes */

class ast_pg_t : public ast_decl_t {
	public:
		ast_pg_t(const std::string& type, const std::string& t) {
			tag = t;
			if (type == "in") {
				fd = 0;
			} else if (type == "out") {
				fd = 1;
			} else {
				throw std::runtime_error("pg type can be 'in' or 'out'!");

			}
		}
		~ast_pg_t() {}

		void set_name(const std::string& n) {
			name = n;
		}
	private:
		std::string name;
		size_t fd;
		std::string tag;
};

class ast_source_t : public ast_node_t {
	public:
		ast_source_t() {}
		void add_decl(const std::shared_ptr<ast_decl_t>& d) {
			decls.push_back(d);
		}
		std::vector<std::shared_ptr<ast_decl_t>>& get_decls() {
			return decls;
		}
		void print() {
			for (auto& d : decls) {
				d.get()->print();
			}
		}
	private:
		std::vector<std::shared_ptr<ast_decl_t>> decls;
};

class ast_decls_t : public ast_node_t {
	public:
		ast_decls_t() {};
		std::vector<std::shared_ptr<ast_decl_t>>& get_decls() {
			return decls;
		}
		void add(const std::shared_ptr<ast_decl_t>& d) {
			decls.push_back(d);
		}
		void print() {
			for (auto& d : decls) {
				d.get()->print();
			}
		}
	private:
		std::vector<std::shared_ptr<ast_decl_t>> decls;
};

class ast_tag_t : public ast_expr_t {
	public:
		ast_tag_t();
		ast_tag_t(const std::string& n) : name(n) {}
		~ast_tag_t() {}
		std::string& get_name() {
			return name;
		}
		void print() {
			std::cout << "\tTag '" << name << "'" << std::endl;
		}
	private:
		std::string name;
};

class ast_edge_t : public ast_node_t {
	public:
		ast_edge_t(const std::string& s, const std::string& e) :
			source(new ast_tag_t(s)), end(new ast_tag_t(e)) {}
		void print() {
			source.get()->print();
			std::cout << "\t--->" << std::endl;
			end.get()->print();
		}
		std::shared_ptr<ast_tag_t>& get_source() {
			return source;
		}
		std::shared_ptr<ast_tag_t>& get_end() {
			return end;
		}
	private:
		std::shared_ptr<ast_tag_t> source;
		std::shared_ptr<ast_tag_t> end;
};

class ast_topology_basic_t : public ast_topology_t {
	public:
		void add_edge(const std::shared_ptr<ast_edge_t>& e) {
			edges.push_back(e);
		}
		std::vector<std::shared_ptr<ast_edge_t>> get_edges() {
			return edges;
		}
		void print() {
			std::cout << "Basic topology '" << name << "'" << std::endl;
			std::cout << "Edges: " << std::endl;
			for (auto& e : edges) {
				e.get()->print();
			}
		}
	private:
		std::vector<std::shared_ptr<ast_edge_t>> edges;
};


class ast_edges_t : public ast_node_t {
	public:
		void print() {
			for (auto& e : edges) {
				e.get()->print();
			}
		}
		std::vector<std::shared_ptr<ast_edge_t>>& get_edges() {
			return edges;
		}
		void add_edge(const std::shared_ptr<ast_edge_t>& e) {
			edges.push_back(e);
		}
	private:
		std::vector<std::shared_ptr<ast_edge_t>> edges;
};

class ast_topology_linear_t : public ast_topology_t {
	public:
		void add_tag(const std::shared_ptr<ast_tag_t>& t) {
			tags.push_back(t);
		}
		std::vector<std::shared_ptr<ast_tag_t>>& get_tags() {
			return tags;
		}
		void print() {
			std::cout << "Linear topology: " << std::endl;
			for (auto& t : tags) {
				t.get()->print();
			}
		}
	private:
		std::vector<std::shared_ptr<ast_tag_t>> tags;
};

class ast_topology_expr_t : public ast_topology_t {
	public:
		ast_topology_expr_t(const std::shared_ptr<ast_expr_t>& e) : expr(e) {}
		void print() {
			std::cout << "Expr topology: '" << name << "'" << std::endl;
			expr->print();
		}
		std::shared_ptr<ast_expr_t>& get_expr() {
			return expr;
		}
	private:
		std::shared_ptr<ast_expr_t> expr;
};

class ast_expr_bin_t : public ast_expr_t {
	public:
		enum class Oper {
			SUM,
			MUL
		};
		ast_expr_bin_t(const Oper o, const std::shared_ptr<ast_expr_t>& l, const std::shared_ptr<ast_expr_t>& r)
			: oper(o), lhs(l), rhs(r) {}

		void set_lhs(const std::shared_ptr<ast_expr_t>& l) {
			lhs = l;
		}
		std::shared_ptr<ast_expr_t>& get_lhs() {
			return lhs;
		}

		void set_rhs(const std::shared_ptr<ast_expr_t>& r) {
			rhs = r;
		}
		std::shared_ptr<ast_expr_t>& get_rhs() {
			return rhs;
		}

		Oper get_oper() {
			return oper;
		}

		void print() {
			std::cout << "\tLeft side:" << std::endl << "\t";
			lhs.get()->print();
			std::cout << "\t\t" << (oper == Oper::MUL ? "*" : "+") << std::endl << "\t";
			std::cout << "\tRight side:" << std::endl << "\t";
			rhs.get()->print();
		}
	private:
		Oper oper;
		std::shared_ptr<ast_expr_t> lhs;
		std::shared_ptr<ast_expr_t> rhs;
};

std::shared_ptr<ast_node_t> ast_construct(const dertree_t& node, std::shared_ptr<ast_node_t> arg);

#endif
