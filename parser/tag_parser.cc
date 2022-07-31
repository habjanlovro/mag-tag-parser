#include "tag_parser.h"

#include <iostream>
#include <fstream>
#include <sstream>


static Tag_type get_type(std::istringstream& iss);
static std::string get_symbol(std::istringstream& iss);
static std::string get_tag(std::istringstream& iss);


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
			std::string symbol = get_symbol(iss);
			std::string tag = get_tag(iss);
			if (policy.contains_tag(tag)) {
				tag_struct_t tag_data = { type, symbol, tag };
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

static std::string get_symbol(std::istringstream& iss) {
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
	if (!colon) {
		do {
			iss.get(c);
		} while (c != ':' && !iss.eof());
	}
	if (iss.eof()) {
		throw std::runtime_error("Missing colon or tag!");
	}
	return r;
}

static std::string get_tag(std::istringstream& iss) {
	std::string r;
	char c;
	do {
		iss.get(c);
	} while (c == ' ' && !iss.eof());
	do {
		r += c;
		iss.get(c);
	} while (!iss.eof());

	std::string::iterator string_end;
	for (auto p = r.begin(); p != r.end(); p++) {
		if (*p != ' ') {
			string_end = p;
		}
	}
	if (string_end != r.end()) {
		string_end++;
	}
	r.erase(string_end, r.end());
	return r;
}
