#ifndef _TAG_PARSER_H_
#define _TAG_PARSER_H_

#include <string>
#include <vector>

#include "policy.h"

enum class Tag_type {
	ATOM,
	PTR
};


typedef struct {
	Tag_type type;
	std::string symbol;
	std::string tag;
	size_t ptr_size;
} tag_struct_t;


class tag_data_t {
	public:
		tag_data_t(const char *file_name, const policy_t& policy);
		~tag_data_t() {}
		const std::vector<tag_struct_t>& getentries() const { return entries; }
	private:
		std::vector<tag_struct_t> entries;
};

#endif
