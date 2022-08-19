#include "lca.h"

#include <list>
#include <iostream>
#include <string>
#include <sstream>
#include <map>


static std::vector<std::vector<uint8_t>> reverse_graph(
	const std::vector<std::vector<uint8_t>>& m);
static std::vector<std::vector<uint8_t>> preprocess_first(
	const std::vector<std::vector<uint8_t>>& m);
static std::vector<std::vector<uint8_t>> preprocess_second(
	const std::vector<std::vector<uint8_t>>& m,
	const std::vector<std::vector<uint8_t>>& desc);

static std::vector<int> get_sources(
	const std::vector<std::vector<uint8_t>>& m,
	const std::list<int>& indexes);
static std::vector<int> get_terminals(
	const std::vector<std::vector<uint8_t>>& m,
	std::list<int> indexes);


std::vector<std::vector<uint8_t>> compute_lca(
		const std::vector<std::vector<uint8_t>>& m) {
	auto transposed = reverse_graph(m);
	auto closure = preprocess_first(transposed);
	return preprocess_second(transposed, closure);
}


// reverse the graph edges by transposing the matrix
static std::vector<std::vector<uint8_t>> reverse_graph(
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

static std::vector<std::vector<uint8_t>> preprocess_first(
		const std::vector<std::vector<uint8_t>>& m) {
	std::vector<std::vector<uint8_t>> r(m.size(), std::vector<uint8_t>(m.size()));

	std::list<int> indexes;
	for (size_t i = 0; i < m.size(); i++) {
		indexes.push_back(i);
	}

	std::vector<int> terminals = get_terminals(m, indexes);
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

		terminals = get_terminals(m, indexes);
	}

	return r;
}


static std::vector<std::vector<uint8_t>> preprocess_second(
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
	std::map<int, uint8_t> number_to_index = {{-1, TAG_INVALID}};
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
		m.size(), std::vector<uint8_t>(m.size(), TAG_INVALID));
	for (size_t i = 0; i < r.size(); i++) {
		for (size_t j = 0; j < r.size(); j++) {
			lca_matrix[i][j] = number_to_index[r[i][j]];
		}
	}

	return lca_matrix;
}


static std::vector<int> get_terminals(
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

static std::vector<int> get_sources(
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
