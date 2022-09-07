#include "synan.h"

#include "lexer.h"

#include <sstream>
#include <exception>


static dertree_t parse_decls(std::vector<symbol_t>& symbols);
static dertree_t parse_decl(std::vector<symbol_t>& symbols);
static dertree_t parse_declrest(std::vector<symbol_t>& symbols);
static dertree_t parse_topology(std::vector<symbol_t>& symbols);
static dertree_t parse_topology_rest(std::vector<symbol_t>& symbols);
static dertree_t parse_basic(std::vector<symbol_t>& symbols);
static dertree_t parse_edge(std::vector<symbol_t>& symbols);
static dertree_t parse_edge_rest(std::vector<symbol_t>& symbols);
static dertree_t parse_linear(std::vector<symbol_t>& symbols);
static dertree_t parse_linear_rest(std::vector<symbol_t>& symbols);
static dertree_t parse_expr(std::vector<symbol_t>& symbols);
static dertree_t parse_sum(std::vector<symbol_t>& symbols);
static dertree_t parse_sum_rest(std::vector<symbol_t>& symbols);
static dertree_t parse_mul(std::vector<symbol_t>& symbols);
static dertree_t parse_mul_rest(std::vector<symbol_t>& symbols);
static dertree_t parse_elem(std::vector<symbol_t>& symbols);
static dertree_t parse_pg(std::vector<symbol_t>& symbols);
static dertree_t parse_pg_rest(std::vector<symbol_t>& symbols);

static inline std::string error_msg(const symbol_t &s, const std::string expected);

static inline symbol_t& peek(std::vector<symbol_t>& symbols, const std::string err);
static inline symbol_t& consume(std::vector<symbol_t>& symbols, const std::string err);

static void add_leaf(dertree_t& t, std::vector<symbol_t>& symbols,
	Term expected_symbol, const std::string& err);


size_t symbol_index = 0;


dertree_t parse_source(std::vector<symbol_t>& symbols) {
	dertree_t t;
	t.label = Nont::SOURCE;
	peek(symbols, "Policy file is empty!");
	t.subtrees.push_back(parse_decls(symbols));
	return t;
}

static dertree_t parse_decls(std::vector<symbol_t>& symbols) {
	dertree_t t;
	t.label = Nont::DECLS;
	t.subtrees.push_back(parse_decl(symbols));
	t.subtrees.push_back(parse_declrest(symbols));
	return t;
}

static dertree_t parse_decl(std::vector<symbol_t>& symbols) {
	dertree_t t;
	t.label = Nont::DECL;
	auto& s = peek(symbols, "Missing declarations!");
	switch (s.term) {
		case Term::TOPOLOGY:
			t.subtrees.push_back(parse_topology(symbols));
			break;
		case Term::PG:
			t.subtrees.push_back(parse_pg(symbols));
			break;
		default:
			throw std::runtime_error(error_msg(s, "declarations"));
	}
	return t;
}

static dertree_t parse_declrest(std::vector<symbol_t>& symbols) {
	dertree_t t;
	t.label = Nont::DECLREST;
	auto& s = peek(symbols, "Missing declarations!");
	switch (s.term) {
		case Term::END:
			break;
		case Term::TOPOLOGY:
		case Term::PG:
			t.subtrees.push_back(parse_decl(symbols));
			t.subtrees.push_back(parse_declrest(symbols));
			break;
		default:
			throw std::runtime_error(error_msg(s, "declarations"));
	}
	return t;
}

static dertree_t parse_topology(std::vector<symbol_t>& symbols) {
	dertree_t t;
	t.label = Nont::TOPOLOGY;

	add_leaf(t, symbols, Term::TOPOLOGY, "'topology'");
	add_leaf(t, symbols, Term::IDENTIFIER, "an identifier");
	add_leaf(t, symbols, Term::COLON, "':'");
	t.subtrees.push_back(parse_topology_rest(symbols));

	return t;
}

static dertree_t parse_topology_rest(std::vector<symbol_t>& symbols) {
	dertree_t t;
	t.label = Nont::TOPOLOGYREST;

	auto& s = consume(symbols, "Missing topology type!");
	t.leaves.push_back(s);

	switch (s.term) {
		case Term::BASIC:
			add_leaf(t, symbols, Term::LBRACE, "'{'");
			t.subtrees.push_back(parse_basic(symbols));
			add_leaf(t, symbols, Term::RBRACE, "'}");
			break;
		case Term::LINEAR:
			t.subtrees.push_back(parse_linear(symbols));
			break;
		case Term::EXPR:
			t.subtrees.push_back(parse_expr(symbols));
			break;
		default:
			std::ostringstream oss;
			oss << "Unsupported topology type '" << s.name << "'! Location: "
				<< s.line << ", " << s.column;
			throw std::runtime_error(oss.str());
	}

	return t;
}

static dertree_t parse_basic(std::vector<symbol_t>& symbols) {
	dertree_t t;
	t.label = Nont::BASIC;
	t.subtrees.push_back(parse_edge(symbols));
	t.subtrees.push_back(parse_edge_rest(symbols));
	return t;
}


static dertree_t parse_edge(std::vector<symbol_t>& symbols) {
	dertree_t t;
	t.label = Nont::EDGE;

	add_leaf(t, symbols, Term::STRING, "a tag string");
	add_leaf(t, symbols, Term::ARROW, "'->'");
	add_leaf(t, symbols, Term::STRING, "a tag string");

	return t;
}

static dertree_t parse_edge_rest(std::vector<symbol_t>& symbols) {
	dertree_t t;
	t.label = Nont::EDGEREST;

	auto& s = peek(symbols, "Missing a ',' or '}'!");
	switch (s.term) {
		case Term::RBRACE:
			break;
		case Term::COMMA:
			t.leaves.push_back(s);
			consume(symbols, "Missing a ','!");
			t.subtrees.push_back(parse_edge(symbols));
			t.subtrees.push_back(parse_edge_rest(symbols));
			break;
		default:
			throw std::runtime_error(error_msg(s, "',' or '}'"));
	}
	return t;
}

static dertree_t parse_linear(std::vector<symbol_t>& symbols) {
	dertree_t t;
	t.label = Nont::LINEAR;

	add_leaf(t, symbols, Term::STRING, "a tag string");
	t.subtrees.push_back(parse_linear_rest(symbols));
	return t;
}

static dertree_t parse_linear_rest(std::vector<symbol_t>& symbols) {
	dertree_t t;
	t.label = Nont::LINEARREST;
	auto& s = peek(symbols, "Missing a ',' or declarations!");
	switch (s.term) {
		case Term::TOPOLOGY:
		case Term::END:
		case Term::PG:
			break;
		case Term::COMMA:
			t.leaves.push_back(s);
			consume(symbols, "Missing a ','!");
			t.subtrees.push_back(parse_linear(symbols));
			break;
		default:
			throw std::runtime_error(error_msg(s, ","));
	}

	return t;
}

static dertree_t parse_expr(std::vector<symbol_t>& symbols) {
	dertree_t t;
	t.label = Nont::EXPR;
	auto& s = peek(symbols, "Missing an identifier or expression!");
	switch (s.term) {
		case Term::IDENTIFIER:
		case Term::LPAREN:
			t.subtrees.push_back(parse_sum(symbols));
			break;
		default:
			throw std::runtime_error(error_msg(s, "an identifier or '('"));
	}

	return t;
}

static dertree_t parse_sum(std::vector<symbol_t>& symbols) {
	dertree_t t;
	t.label = Nont::SUM;
	auto& s = peek(symbols, "Missing identifier or expression!");
	switch (s.term) {
		case Term::IDENTIFIER:
		case Term::LPAREN:
			t.subtrees.push_back(parse_mul(symbols));
			t.subtrees.push_back(parse_sum_rest(symbols));
			break;
		default:
			throw std::runtime_error(error_msg(s, "an identifier or '('"));
	}

	return t;
}

static dertree_t parse_sum_rest(std::vector<symbol_t>& symbols) {
	dertree_t t;
	t.label = Nont::SUMREST;
	auto& s = peek(symbols, "Missing end of expression or '+'!");
	switch (s.term) {
		case Term::TOPOLOGY:
		case Term::RPAREN:
		case Term::END:
		case Term::PG:
			break;
		case Term::PLUS:
			t.leaves.push_back(s);
			consume(symbols, "Missing a '+'!");
			t.subtrees.push_back(parse_mul(symbols));
			t.subtrees.push_back(parse_sum_rest(symbols));
			break;
		default:
			throw std::runtime_error(error_msg(s, " end of expression or '+'"));
	}

	return t;
}

static dertree_t parse_mul(std::vector<symbol_t>& symbols) {
	dertree_t t;
	t.label = Nont::MUL;
	auto& s = peek(symbols, "Missing identifier or expression!");
	switch (s.term) {
		case Term::IDENTIFIER:
		case Term::LPAREN:
			t.subtrees.push_back(parse_elem(symbols));
			t.subtrees.push_back(parse_mul_rest(symbols));
			break;
		default:
			throw std::runtime_error(error_msg(s, "an identifier or '('"));
	}
	return t;
}

static dertree_t parse_mul_rest(std::vector<symbol_t>& symbols) {
	dertree_t t;
	t.label = Nont::MULREST;
	auto& s = peek(symbols, "Missing end of expression!");
	switch (s.term) {
		case Term::TOPOLOGY:
		case Term::RPAREN:
		case Term::END:
		case Term::PLUS:
		case Term::PG:
			break;
		case Term::MULT:
			t.leaves.push_back(s);
			consume(symbols, "Missing a '*'");
			t.subtrees.push_back(parse_elem(symbols));
			t.subtrees.push_back(parse_mul_rest(symbols));
			break;
		default:
			throw std::runtime_error(error_msg(s, " end of expression or '*'"));
	}
	return t;
}

static dertree_t parse_elem(std::vector<symbol_t>& symbols) {
	dertree_t t;
	t.label = Nont::ELEM;
	auto& s = consume(symbols, "Missing an identifier or a nested expression!");
	switch (s.term) {
		case Term::IDENTIFIER:
			t.leaves.push_back(s);
			break;
		case Term::LPAREN:
			t.leaves.push_back(s);
			t.subtrees.push_back(parse_sum(symbols));
			add_leaf(t, symbols, Term::RPAREN, "')'");
			break;
		default:
			throw std::runtime_error(error_msg(s, "an identifier or a nested expression"));
	}
	return t;
}

static dertree_t parse_pg(std::vector<symbol_t>& symbols) {
	dertree_t t;
	t.label = Nont::PG;

	add_leaf(t, symbols, Term::PG, "'pg'");
	add_leaf(t, symbols, Term::IDENTIFIER, "an identifier");
	add_leaf(t, symbols, Term::LBRACE, "'{'");
	t.subtrees.push_back(parse_pg_rest(symbols));
	add_leaf(t, symbols, Term::RBRACE, "'}'");

	return t;
}

static dertree_t parse_pg_rest(std::vector<symbol_t>& symbols) {
	dertree_t t;
	t.label = Nont::PG_REST;

	auto& s = consume(symbols, "Missing an identifier or a nested expression!");
	switch (s.term) {
		case Term::PG_TYPE:
			t.leaves.push_back(s);
			add_leaf(t, symbols, Term::COLON, "':'");
			add_leaf(t, symbols, Term::IDENTIFIER, "'in', 'out' or 'err'");
			if ((t.leaves.back().name != "in") &&
					(t.leaves.back().name != "out") &&
					(t.leaves.back().name != "err")) {
				throw std::runtime_error(error_msg(t.leaves.back(), "'in', out' or 'err"));
			}
			break;
		case Term::PG_FILE:
			t.leaves.push_back(s);
			add_leaf(t, symbols, Term::COLON, "':'");
			add_leaf(t, symbols, Term::STRING, "a filename");
			break;
		default:
			throw std::runtime_error(error_msg(s, "'type' or 'file' keywords!"));
	}

	add_leaf(t, symbols, Term::IDENTIFIER, "'tag'");
	if (t.leaves.back().name != "tag") {
		throw std::runtime_error(error_msg(t.leaves.back(), "'tag'"));
	}
	add_leaf(t, symbols, Term::EQUAL, "'='");
	add_leaf(t, symbols, Term::STRING, "a string");

	return t;
}


static inline std::string error_msg(const symbol_t &s, const std::string expected) {
	std::ostringstream oss;
	oss << "Expected " << expected << ", got '" << s.name << "'! Location: "
		<< s.line << ", " << s.column;
	return oss.str();
}

static inline symbol_t& peek(std::vector<symbol_t>& symbols, const std::string err) {
	try {
		return symbols.at(symbol_index);
	} catch (std::out_of_range& e) {
		throw std::runtime_error(err);
	}
}

static inline symbol_t& consume(std::vector<symbol_t>& symbols, const std::string err) {
	try {
		symbol_index++;
		return symbols.at(symbol_index - 1);
	} catch (std::out_of_range& e) {
		throw std::runtime_error(err);
	}
}

static void add_leaf(dertree_t& t, std::vector<symbol_t>& symbols,
		Term expected_symbol, const std::string& err) {
	auto& s = consume(symbols, "Missing " + err);
	if (s.term != expected_symbol) {
		throw std::runtime_error(error_msg(s, err));
	}
	t.leaves.push_back(s);
}
