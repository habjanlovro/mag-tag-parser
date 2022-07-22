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

static inline std::string error_msg(const symbol_t &s, const std::string expected);

static inline symbol_t& peek(std::vector<symbol_t>& symbols, const std::string err);
static inline symbol_t& consume(std::vector<symbol_t>& symbols, const std::string err);


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
	t.subtrees.push_back(parse_topology(symbols));
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
	symbol_t &s = consume(symbols, "Missing 'topology'!");
	if (s.term != Term::TOPOLOGY) {
		throw std::runtime_error(error_msg(s, "'topology'"));
	}
	t.leaves.push_back(s);

	s = consume(symbols, "Missing an identifier!");
	if (s.term != Term::IDENTIFIER) {
		throw std::runtime_error(error_msg(s, "an identifier"));
	}
	t.leaves.push_back(s);

	s = consume(symbols, "Missing a ':'!");
	if (s.term != Term::COLON) {
		throw std::runtime_error(error_msg(s, "':'"));
	}
	t.leaves.push_back(s);

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
			s = consume(symbols, "Missing '{'!");
			t.leaves.push_back(s);
			t.subtrees.push_back(parse_basic(symbols));
			s = consume(symbols, "Missing '}'!");
			t.leaves.push_back(s);
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

	auto& s = consume(symbols, "Missing an identifier!");
	if (s.term != Term::IDENTIFIER) {
		throw std::runtime_error(error_msg(s, "an identifier"));
	}
	t.leaves.push_back(s);

	s = consume(symbols, "Missing '->'!");
	if (s.term != Term::ARROW) {
		throw std::runtime_error(error_msg(s, "'->'"));
	}
	t.leaves.push_back(s);

	s = consume(symbols, "Missing an identifier!");
	if (s.term != Term::IDENTIFIER) {
		throw std::runtime_error(error_msg(s, "an identifier"));
	}
	t.leaves.push_back(s);

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

	auto& s = consume(symbols, "Missing an identifier!");
	if (s.term != Term::IDENTIFIER) {
		throw std::invalid_argument(error_msg(s, "an identifier"));
	}
	t.leaves.push_back(s);
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
			s = consume(symbols, "Missing ')'!");
			t.leaves.push_back(s);
			break;
		default:
			throw std::runtime_error(error_msg(s, "an identifier or a nested expression"));
	}
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
