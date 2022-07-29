#include <iostream>
#include <errno.h>
#include <cstring>
#include "elf.h"
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <memory>

#include "elf_parser.h"
#include "tag_parser.h"

#include "policy.h"


void out_print_line(const uint64_t addr, const size_t size, const std::string tag) {
	std::cout << "0x" << std::hex << addr << "," << size << "," << tag << std::endl;
}


void check_stuff(elf_data_t *elf_data, tag_data_t *tag_data) {
	for (auto &tag_entry : tag_data->getentries()) {
		try {
			elf_symbol_t elf_symbol = elf_data->get_symbol_info(tag_entry.symbol);
			if (tag_entry.type == Tag_type::PTR) {
				uint64_t addr = elf_data->get_ptr_addr(elf_symbol.value);
				if (addr > 0) {
					out_print_line(addr, 1, tag_entry.tag);
				}
			}
			out_print_line(elf_symbol.value, elf_symbol.size, tag_entry.tag);
		} catch (std::runtime_error& e) {
			std::cerr << e.what() << std::endl;
		}
	}
}


int main(int argc, char *argv[]) {
	if (argc < 4) {
		std::cout << "Missing arguments!" << std::endl;
		std::cout << "Usage: " << argv[0] << " <elf-file> <tag-file> <policy-file>" << std::endl;
		return 0;
	}

	std::unique_ptr<policy_t> policy;

	try {
		policy = std::make_unique<policy_t>(argv[3]);
	} catch (std::runtime_error& err) {
		std::cout << err.what() << std::endl;
	} catch (...) {
		std::cout << "Failed policy!" << std::endl;
	}

	std::unique_ptr<elf_data_t> elf_data;
	std::unique_ptr<tag_data_t> tag_data;

	try {
		elf_data = std::make_unique<elf_data_t>(argv[1]);
	} catch (std::exception& e) {
		std::cerr << "exception: " << e.what() << std::endl;
		exit(1);
	} catch (...) {
		std::cerr << "Failed to get ELF data!" << std::endl;
		exit(1);
	}

	try {
		tag_data = std::make_unique<tag_data_t>(argv[2], *policy);
	} catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
		exit(1);
	} catch (...) {
		std::cerr << "Failed to get tag data!" << std::endl;
		exit(1);
	}

	check_stuff(elf_data.get(), tag_data.get());

	return 0;
}
