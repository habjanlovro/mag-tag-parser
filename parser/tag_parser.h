#ifndef _TAG_PARSER_H_
#define _TAG_PARSER_H_

#include <string>
#include <vector>


enum tag_type {
	ATOM,
	PTR
};


typedef struct {
	enum tag_type type;
	std::string symbol;
	std::string tag;
} tag_struct_t;


class tag_data_t {
	public:
		tag_data_t(const char *file_name);
		~tag_data_t() {}
		const std::vector<tag_struct_t>& getentries() const { return entries; }
	private:
		std::vector<tag_struct_t> entries;
};

#endif
