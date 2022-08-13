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

#define INVALID_TAG ((uint8_t) ((1 << 8) - 1))

const std::string output_file_name = "policy.d2sc";

void print_policy(std::ofstream& out, const policy_t& policy);
void print_tags(
		std::ofstream& out,
		const elf_data_t& elf_data,
		const tag_data_t& tag_data,
		const policy_t& policy);
static inline void out_print_line(std::ofstream& out, const uint64_t addr,
	const size_t size, const int tag_index);


void topological_sort_dfs(
		const std::vector<std::vector<uint8_t>>& m,
		const int index,
		std::vector<bool>& discovered,
		std::vector<int>& end_time,
		int& time,
		std::list<int>& topological_order) {
	discovered[index] = true;
	for (size_t j = 0; j < m[index].size(); j++) {
		if (m[index][j] > 0 && !discovered[j]) {
			topological_sort_dfs(m, j, discovered, end_time, time, topological_order);
		}
	}
	end_time[index] = time;
	topological_order.push_front(index);
	time++;
}

std::list<int> topological_ordering(const std::vector<std::vector<uint8_t>>& m) {
	std::vector<bool> discovered(m.size());
	std::vector<int> end_time(m.size());
	int time = 0;
	std::list<int> topological_order;

	for (size_t i = 0; i < m.size(); i++) {
		if (!discovered[i]) {
			topological_sort_dfs(m, i, discovered, end_time, time, topological_order);
		}
	}

	for (size_t i = 0; i < m.size(); i++) {
		for (size_t j = 0; j < m.size(); j++) {
			if (i != j && m[i][j] > 0 && end_time[i] <= end_time[j]) {
				std::ostringstream oss;
				oss << "The policy is not a directed acyclical graph!";// <<
					// " A cycle was detected with tags '" << t.get_tag(i) << "'"
					// << " -> '" << t.get_tag(j) << "'!";
				throw std::runtime_error(oss.str());
			}
		}
	}
	return topological_order;
}


// reverse the graph edges by transposing the matrix
std::vector<std::vector<uint8_t>> reverse_graph(
		const std::vector<std::vector<uint8_t>>& m) {
	auto rm = std::vector<std::vector<uint8_t>>(
		m.size(), std::vector<uint8_t>(m.size()));

	for (size_t i = 0; i < m.size(); i++) {
		for (size_t j = 0; j < m.size(); j++) {
			rm[j][i] = m[i][j];
		}
	}

	return rm;
}

void add_virtual_node(std::vector<std::vector<uint8_t>>& m) {
	std::vector<uint8_t> virtual_edges(m.size() + 1);
	for (size_t j = 0; j < m.size(); j++) {
		int in_degree = 0;
		for (size_t i = 0; i < m.size(); i++) {
			in_degree += m[i][j];
		}
		if (in_degree <= 1) {
			virtual_edges[j] = 1;
		}
		m[j].push_back(0);
	}
	m.push_back(virtual_edges);
}


std::vector<int> find_terminals(
		const std::vector<std::vector<uint8_t>>& m,
		std::list<int> indexes) {
	std::vector<int> r;
	for (auto& i : indexes) {
		bool terminal = true;
		for (auto& j : indexes) {
			if (i != j && m[i][j] > 0) {
				terminal = false;
				break;
			}
		}
		if (terminal) {
			r.push_back(i);
		}
	}
	return r;
}


std::vector<std::vector<uint8_t>> preprocess_first(const std::vector<std::vector<uint8_t>>& m) {
	std::vector<std::vector<uint8_t>> r(m.size(), std::vector<uint8_t>(m.size()));

	std::list<int> indexes;
	for (size_t i = 0; i < m.size(); i++) {
		indexes.push_back(i);
	}

	std::vector<int> terminals = find_terminals(m, indexes);
	std::vector<int> last_deleted;
	while (terminals.size() > 0) {
		for (auto& i : terminals) {
			r[i][i] = 1;
			for (auto& row : last_deleted) {
				if (m[i][row] > 0) {
					for (size_t j = 0; j < r.size(); j++) {
						r[i][j] |= r[row][j];
					}
				}
			}
			indexes.remove(i);
		}
		last_deleted.clear();
		last_deleted = terminals;

		terminals = find_terminals(m, indexes);
	}

	return r;
}

std::vector<int> get_sources(
		const std::vector<std::vector<uint8_t>>& m,
		const std::list<int>& indexes) {
	std::vector<int> r;
	for (auto& i : indexes) {
		bool source = true;
		for (auto& j : indexes) {
			if (i != j && m[j][i] > 0) {
				source = false;
				break;
			}
		}
		if (source) {
			r.push_back(i);
		}
	}
	return r;
}

std::vector<std::vector<uint8_t>> preprocess_second(
		const std::vector<std::vector<uint8_t>>& m,
		const std::vector<std::vector<uint8_t>>& desc) {
	std::vector<std::vector<int>> r(m.size(), std::vector<int>(m.size(), -1));

	std::list<int> indexes;
	for (size_t i = 0; i < m.size(); i++) {
		indexes.push_back(i);
	}
	std::list<int> deleted;
	std::vector<int> sources = get_sources(m, indexes);
	int numbering = 0;
	std::map<int, uint8_t> number_to_index = {{-1, INVALID_TAG}};
	while (sources.size() > 0) {
		for (auto& s : sources) {
			number_to_index[numbering] = s;
			for (size_t j = 0; j < r.size(); j++) {
				if (desc[s][j] > 0) {
					r[s][j] = numbering;
				}
			}
			for (auto& ancestor : deleted) {
				if (m[ancestor][s] > 0) {
					for (size_t j = 0; j < r.size(); j++) {
						if (r[ancestor][j] > r[s][j]) {
							r[s][j] = r[ancestor][j];
						}
					}
				}
			}

			deleted.push_back(s);
			indexes.remove(s);
			numbering++;
		}

		sources = get_sources(m, indexes);
	}

	std::vector<std::vector<uint8_t>> lca_matrix(
		m.size(), std::vector<uint8_t>(m.size(), INVALID_TAG));
	for (size_t i = 0; i < r.size(); i++) {
		for (size_t j = 0; j < r.size(); j++) {
			lca_matrix[i][j] = number_to_index[r[i][j]];
		}
	}

	return lca_matrix;
}


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
		auto transposed = reverse_graph(matrix);
		auto closure = preprocess_first(transposed);
		auto lca_matrix = preprocess_second(transposed, closure);
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
					out_print_line(out, addr, 1, policy.tag_index(tag_entry.tag));
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
