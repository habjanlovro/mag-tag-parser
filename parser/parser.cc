#include <iostream>
#include <fstream>
#include <errno.h>
#include <cstring>
#include "elf.h"
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <memory>
#include <sstream>
#include <list>

#include "elf_parser.h"
#include "tag_parser.h"

#include "policy.h"
#include "lca.h"

const std::string output_file_name = "policy.d2sc";

void print_policy(std::ofstream& out, const policy_t& policy);
void print_tags(
		std::ofstream& out,
		const elf_data_t& elf_data,
		const tag_data_t& tag_data,
		const policy_t& policy);
static inline void out_print_line(std::ofstream& out, const uint64_t addr,
	const size_t size, const int tag_index);


int main(int argc, char *argv[]) {
	if (argc < 4) {
		std::cout << "Missing arguments!" << std::endl;
		std::cout << "Usage: " << argv[0] << " <elf-file> <tag-file> <policy-file>" << std::endl;
		return 0;
	}

	std::unique_ptr<policy_t> policy;
	std::unique_ptr<elf_data_t> elf_data;
	std::unique_ptr<tag_data_t> tag_data;

	try {
		policy = std::make_unique<policy_t>(argv[3]);
		auto& matrix = policy->topology->matrix();
		auto lca_matrix = compute_lca(matrix);
		policy->set_lca_matrix(lca_matrix);
	} catch (std::runtime_error& err) {
		std::cerr << err.what() << std::endl;
		exit(1);
	} catch (...) {
		std::cerr << "Failed policy!" << std::endl;
		exit(1);
	}

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

	std::ofstream out_file(output_file_name);
	if (out_file.is_open()) {
		print_policy(out_file, *policy);
		print_tags(out_file, *elf_data, *tag_data, *policy);
	}

	return 0;
}

void print_policy(std::ofstream& out, const policy_t& policy) {
	out << policy.topology->size() << std::endl;
	auto& m = policy.get_lca_matrix();
	for (size_t i = 0; i < m.size(); i++) {
		out << policy.topology->get_tag(i);
		for (size_t j = 0; j < m[i].size(); j++) {
			out << " " << (int) m[i][j];
		}
		out << std::endl;
	}
}

void print_tags(
		std::ofstream& out,
		const elf_data_t& elf_data,
		const tag_data_t& tag_data,
		const policy_t& policy) {
	for (auto &tag_entry : tag_data.getentries()) {
		try {
			elf_symbol_t elf_symbol = elf_data.get_symbol_info(tag_entry.symbol);
			if (tag_entry.type == Tag_type::PTR) {
				uint64_t addr = elf_data.get_ptr_addr(elf_symbol.value);
				if (addr > 0) {
					out_print_line(out, addr, tag_entry.ptr_size, policy.tag_index(tag_entry.tag));
				}
			}
			out_print_line(out, elf_symbol.value, elf_symbol.size, policy.tag_index(tag_entry.tag));
		} catch (std::runtime_error& e) {
			std::cerr << e.what() << std::endl;
		}
	}
}

static inline void out_print_line(std::ofstream& out, const uint64_t addr,
		const size_t size, const int tag_index) {
	out << "0x" << std::hex << addr << ","
		<< std::dec << size << "," << tag_index << std::endl;
}
