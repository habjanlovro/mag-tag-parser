#ifndef _ELF_PARSER_H_
#define _ELF_PARSER_H_

#include <vector>
#include <string>
#include <map>

#include "elf.h"


typedef struct {
	std::string name;
	uint8_t type;
	uint8_t bind;
	uint8_t visibility;
	uint16_t section_index;
	uint64_t value;
	uint64_t size;
} elf_symbol_t;

typedef struct {
	std::string name;
	Elf64_Shdr shdr;
} elf_shdr_t;


class elf_data_t {
	public:
		elf_data_t(const char *file_path);
		~elf_data_t();
		void print_symbols();
		elf_symbol_t get_symbol_info(std::string name);
		uint64_t get_ptr_addr(uint64_t ptr);

	private:
		int fd;
		std::vector<elf_shdr_t> section_hdrs;
		Elf64_Ehdr ehdr;
		std::map<std::string, elf_symbol_t> symbol_table;
};



#endif /* _ELF_PARSER_H_ */
