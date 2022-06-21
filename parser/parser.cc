#include <iostream>
#include <errno.h>
#include <cstring>
#include "elf.h"
#include <unistd.h>
#include <fcntl.h>
#include <vector>

#include "elf_parser.h"

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("Missing arguments!\n");
		printf("Usage: %s <elf-file>\n", argv[0]);
		return 0;
	}

	try {
		elf_data_t elf_data(argv[1]);
		elf_data.print_symbols();
	} catch (...) {
		std::cerr << "Failed to get ELF data!" << std::endl;
		return 1;
	}
	return 0;
}
