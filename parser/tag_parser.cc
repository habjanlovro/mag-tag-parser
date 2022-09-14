#include "tag_parser.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>


static Tag_type get_type(std::istringstream& iss);
static std::pair<std::string, bool> get_symbol(std::istringstream& iss);
static std::string get_tag(std::istringstream& iss, bool colon);
static size_t get_ptr_size(std::istringstream& iss, bool& colon);

tag_data_t::tag_data_t(const char *file_path, const policy_t& policy) {
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
		try {

			Tag_type type = get_type(iss);
			auto t = get_symbol(iss);
			std::string symbol = t.first;
			size_t size = (type == Tag_type::PTR) ?
				get_ptr_size(iss, t.second) :
				0;
			std::string tag = get_tag(iss, t.second);

			if (policy.contains_tag(tag)) {
				tag_struct_t tag_data = { type, symbol, tag, size };
				entries.push_back(tag_data);
			} else {
				std::cerr << "Tag '" << tag << "' is not in the specified policy!"
					<< std::endl;
			}
		} catch (std::runtime_error& err) {
			std::ostringstream oss;
			oss << "Line " << line_num << ": Wrong syntax! " << err.what();
			throw std::invalid_argument(oss.str());
		}
		line_num++;
	}
}

static Tag_type get_type(std::istringstream& iss) {
	std::string r;
	char c;
	while (iss.get(c)) {
		if (c == ' ') {
			break;
		}
		r += c;
	}
	if (r == "ptr") {
		return Tag_type::PTR;
	} else if (r == "atom") {
		return Tag_type::ATOM;
	}
	throw std::runtime_error("Only 'ptr' or 'atom' keywords allowed!");
}

static std::pair<std::string, bool> get_symbol(std::istringstream& iss) {
	std::string r;
	char c;
	bool colon = false;
	while (iss.get(c)) {
		if (c == ':') {
			colon = true;
			break;
		} else if (c == ' ') {
			break;
		}
		r += c;
	}

	if (iss.eof()) {
		throw std::runtime_error("Missing rest of tag declaration!");
	}
	return std::make_pair(r, colon);
}

static std::string get_tag(std::istringstream& iss, bool colon) {
	std::string r;
	char c;
	if (!colon) {
		bool colon_found = false;
		do {
			iss.get(c);
			colon_found = c == ':';
		} while (!colon_found && !iss.eof());

		if (!colon_found) {
			throw std::runtime_error("Colon not found in declaration!");
		}
	}

	do {
		iss.get(c);
	} while (c != '"' && !iss.eof());
	while(iss.get(c) && c != '"') {
		r += c;
	}
	if (c != '"') {
		throw std::runtime_error("Missing end of tag declaration '\"'!");
	}
	r.erase(std::remove_if(r.begin(), r.end(), isspace), r.end());
	if (r.size() == 0) {
		throw std::runtime_error("Missing tag in declaration!");
	}
	return r;
}


static size_t get_ptr_size(std::istringstream& iss, bool& colon) {
	if (colon) {
		throw std::runtime_error("Pointer declaration needs size argument!");
	}
	char c;
	size_t r = 0;
	std::string size_word;
	for (int i = 0; i < 4; i++) {
		iss.get(c);
		size_word += c;
	}
	if (size_word != "size") {
		throw std::runtime_error("Expected 'size' keyword!");
	}

	bool found = false;
	do {
		iss.get(c);
		found = c == '=';
	} while (!found && !iss.eof());

	if (!found) {
		throw std::runtime_error("Missing '=' sign in declaration!");
	}

	size_word.clear();
	do {
		iss.get(c);
	} while (c == ' ');

	size_word += c;
	while (iss.get(c)) {
		if (c == ' ') {
			break;
		}
		if (c == ':') {
			colon = true;
			break;
		}
		size_word += c;
	}
	r = std::stoul(size_word);

	return r;
}
