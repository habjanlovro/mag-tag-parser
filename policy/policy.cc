#include "policy.h"

#include "lexer.h"
#include "synan.h"

#include <iostream>

void print_tree(dertree_t& t) {
	for (auto &s : t.leaves) {
		std::cout << s.name << " ";
	}
	std::cout << std::endl;
	for (auto &st : t.subtrees) {
		print_tree(st);
	}
}

void handle_policy(const char *file_path) {
	std::vector<symbol_t> symbols = lexify(file_path);

	try {
		dertree_t tree = parse_source(symbols);
		print_tree(tree);
	} catch (std::runtime_error& err) {
		std::cerr << err.what() << std::endl;
		exit(1);
	}
}
