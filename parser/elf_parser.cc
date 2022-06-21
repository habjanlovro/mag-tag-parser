#include "elf_parser.h"

#include <unistd.h>
#include <errno.h>
#include <iostream>
#include <cstring>
#include <fcntl.h>
#include <exception>

static inline bool elf_check_file(Elf64_Ehdr *hdr) {
    return hdr &&
		hdr->e_ident[EI_MAG0] == ELFMAG0 &&
		hdr->e_ident[EI_MAG1] == ELFMAG1 &&
		hdr->e_ident[EI_MAG2] == ELFMAG2 &&
		hdr->e_ident[EI_MAG3] == ELFMAG3;
}

static inline bool elf_is64(Elf64_Ehdr *hdr) {
	return hdr->e_ident[4] == ELFCLASS64;
}

static inline bool elf_is_riscv(Elf64_Ehdr *hdr) {
	return hdr->e_machine == EM_RISCV;
}

elf_data_t::elf_data_t(const char *file_name) :
		fd(-1) {
	fd = open(file_name, O_RDONLY);
	if (fd < 0) {
		std::cerr << "Unable to open " << file_name << "! Error: " << strerror(errno) << std::endl;
		throw;
	}

	if(pread(fd, &ehdr, sizeof(ehdr), 0) < 0) {
		std::cerr << "fread: " << strerror(errno) << std::endl;
		close(fd);
		throw;
	}

	if (!elf_check_file(&ehdr) || !elf_is64(&ehdr) || !elf_is_riscv(&ehdr)) {
		close(fd);
		throw;
	}

	for (int i = 0; i < ehdr.e_shnum; i++) {
		Elf64_Shdr shdr;
		if (pread(fd, &shdr, ehdr.e_shentsize, sizeof(Elf64_Ehdr) + ehdr.e_shoff + i * sizeof(Elf64_Shdr)) < 0) {
			std::cerr << "Failed section pread: " << strerror(errno) << std::endl;
			continue;
		}
		sections.push_back(shdr);
	}

	for (auto& shdr : sections) {
		if (shdr.sh_type == SHT_SYMTAB) {
			Elf64_Sym *sym_table = (Elf64_Sym *) malloc(shdr.sh_size);
			if (sym_table == NULL) {
				close(fd);
				throw;
			}
			if (pread(fd, sym_table, shdr.sh_size, shdr.sh_offset) < 0) {
				std::cerr << "Sym table fread: " << strerror(errno) << std::endl;
				break;
			}

			Elf64_Shdr linked_section = sections.at(shdr.sh_link - 1);
			char *str_table = (char *) malloc(linked_section.sh_size);
			if (pread(fd, str_table, linked_section.sh_size, linked_section.sh_offset) < 0) {
				std::cerr << "Linked section fread: " << strerror(errno) << std::endl;
				break;
			}

			int entries = shdr.sh_size / sizeof(Elf64_Sym);
			for (int i = 0; i < entries; i++) {
				std::string name(str_table + sym_table[i].st_name);
				elf_symbol_t symbol = {name, ELF64_ST_TYPE(sym_table[i].st_info),
					ELF64_ST_BIND(sym_table[i].st_info), sym_table[i].st_other,
					sym_table[i].st_shndx, sym_table[i].st_value,
					sym_table[i].st_size};
				symbol_table[name] = symbol;
			}

			break;
		}
	}
}

elf_data_t::~elf_data_t() {
	if (fd > 0) {
		if (close(fd) < 0) {
			std::cerr << "Failed to close file descriptor! Error: " << strerror(errno) << std::endl;
		}
	}
}

void elf_data_t::print_symbols() {
	for (auto & symbol : symbol_table) {
		std::cout << symbol.first << std::endl;
	}
}
