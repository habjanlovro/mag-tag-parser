#include "policy.h"
#include "lexer.h"
void handle_policy(const char *file_path) {
	std::vector<symbol_t> symbols = lexify(file_path);

}
