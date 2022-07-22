#include "policy.h"

#include "lexer.h"
#include "synan.h"
#include "ast.h"

#include <iostream>


void handle_policy(const char *file_path) {
	std::vector<symbol_t> symbols = lexify(file_path);

	try {
		dertree_t tree = parse_source(symbols);

		std::shared_ptr<ast_node_t> ast = ast_construct(tree, nullptr);

		ast->print();
	} catch (std::runtime_error& err) {
		std::cerr << err.what() << std::endl;
		exit(1);
	}
}
