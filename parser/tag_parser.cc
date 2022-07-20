#include "tag_parser.h"

#include <iostream>
#include <fstream>
#include <sstream>


tag_data_t::tag_data_t(const char *file_path) {
	std::ifstream infile(file_path);

	if (!infile.is_open()) {
		std::ostringstream oss;
		oss << "Couldn't open tag file: '" << file_path << "'!";
		throw std::invalid_argument(oss.str());
	}

	std::string line;
	int line_num = 1;
	while (std::getline(infile, line)) {
		if (line.size() == 0) { // skip empty lines
			continue;
		}
		std::istringstream iss(line);
		std::string type, symbol, colon, tag;
    	if (!(iss >> type >> symbol >> colon >> tag)) {
			throw std::invalid_argument("Line " + std::to_string(line_num) + ": Wrong syntax!");
		}
		if (colon != ":") {
			throw std::invalid_argument("Line " + std::to_string(line_num) + ": Wrong syntax - no colon!");;
		}
		enum tag_type ttype;
		if (type == "atom") {
			ttype = ATOM;
		} else if (type == "ptr") {
			ttype = PTR;
		} else {
			throw std::invalid_argument("Line " + std::to_string(line_num) + ": Wrong syntax - tag type must be 'atom' or 'ptr'!");
		}

		tag_struct_t tag_data = { ttype, symbol, tag };
		entries.push_back(tag_data);

		line_num++;
	}
}
