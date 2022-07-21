#include "lexer.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <exception>


static inline bool is_whitespace(const char c);
static inline bool is_identifier(const char c);

std::vector<symbol_t> lexify(const char *file_path) {
	std::ifstream infile(file_path);
	char c = 0;
	std::vector<symbol_t> symbols;

	if (!infile.is_open()) {
		std::ostringstream oss;
		oss << "Couldn't open policy file: '" << file_path << "'!";
		throw std::invalid_argument(oss.str());
	}

	int line = 1;
	int column = 0;
	while (infile.get(c)) {
		column++;

		if (is_whitespace(c)) {
			if (c == '\t') {
				column = column + 8 - (column % 8);
			} else if (c == '\n') {
				line += 1;
				column = 0;
			}
			continue;
		} else if (c == '#') {
			do {
				infile.get(c);
			} while (c != '\n');
			line++;
			column = 0;
		} else if (c == '{') {
			symbols.push_back({ Term::LBRACE, "{", line, column });
		} else if (c == '}') {
			symbols.push_back({ Term::RBRACE, "}", line, column });
		} else if (c == '(') {
			symbols.push_back({ Term::LPAREN, "(", line, column });
		} else if (c == ')') {
			symbols.push_back({ Term::RPAREN, ")", line, column });
		} else if (c == ':') {
			symbols.push_back({ Term::COLON, ":", line, column });
		} else if (c == ',') {
			symbols.push_back({ Term::COMMA, ",", line, column });
		} else if (c == '+') {
			symbols.push_back({ Term::PLUS, "+", line, column });
		} else if (c == '*') {
			symbols.push_back({ Term::MULT, "*", line, column });
		} else if (c == '-') {
			infile.get(c);
			column++;
			if (c == '>') {
				symbols.push_back({ Term::ARROW, "->", line, column - 1 });
			} else {
				std::ostringstream oss;
				oss << "Expected an arrow, got '" << c << "', location: "
					<< line << ", " << column;
				throw std::runtime_error(oss.str());
			}
		} else if ((c >= 'a' && c <= 'z') ||
				(c >= 'A' && c <= 'Z') ||
				(c >= '0' && c <= '9')) {
			std::string s;
			int start_column = column;
			do {
				s += c;
				infile.get(c);
				column++;
			} while (!is_whitespace(c) && is_identifier(c));
			infile.unget();
			column--;

			if (s == "basic") {
				symbols.push_back({ Term::BASIC, s, line, start_column });
			} else if (s == "linear") {
				symbols.push_back({ Term::LINEAR, s, line, start_column });
			} else if (s == "expr") {
				symbols.push_back({ Term::EXPR, s, line, start_column });
			} else if (s == "topology") {
				symbols.push_back({ Term::TOPOLOGY, s, line, start_column });
			} else if (s == "pg") {
				symbols.push_back({ Term::PG, s, line, start_column });
			} else {
				symbols.push_back({ Term::IDENTIFIER, s, line, start_column });
			}
		} else {
			std::ostringstream oss;
			oss << "Failed to parse at character '" << c << "', location: "
				<< line << ", " << column;
			throw std::runtime_error(oss.str());
		}
	}

	symbols.push_back({ Term::END, "end of file", line, column });

	for (auto & s : symbols) {
		std::cout << s.name << " loc: " << s.line << ", " << s.column << std::endl;
	}

	return symbols;
}

static inline bool is_whitespace(const char c) {
	return c == '\t' || c == '\n' || c == ' ' || c == '\r';
}

static inline bool is_identifier(const char c) {
	return (c >= 'a' && c <= 'z') ||
		(c >= 'A' && c <= 'Z') ||
		(c >= '0' && c <= '9') ||
		(c == '_');
}
