#ifndef _POLICY_LEXER_H_
#define _POLICY_LEXER_H_


#include <vector>
#include <string>

enum class Term {
	LBRACE,
	RBRACE,
	LPAREN,
	RPAREN,
	PLUS,
	MULT,
	COLON,
	COMMA,
	ARROW,
	BASIC,
	LINEAR,
	EXPR,
	TOPOLOGY,
	PG,
	IDENTIFIER,
	END
};

struct symbol_t {
	Term term;
	std::string name;
	int line;
	int column;
};


std::vector<symbol_t> lexify(const char *file_path);

#endif