#include <vector>
#include <iostream>
#include <set>

struct tree_t {
	int index;
	int depth;
	tree_t *parent;
	std::vector<tree_t *> children;

	tree_t(int index, int depth, tree_t *parent, std::vector<tree_t*> children)
			: index(index), depth(depth), parent(parent), children(children) {}
};

bool is_tree_node(const std::vector<std::vector<uint8_t>>& m, const int index, std::vector<int>& parents) {
	int in_degree = 0;
	for (size_t i = 0; i < m.size(); i++) {
		std::cout << (int) m[i][index] << " ";
		if (i != index && m[i][index] > 0) {
			in_degree++;
			parents.push_back(i);
		}
	}
	std::cout << std::endl;
	return in_degree == 1;
}

bool add_node(tree_t *t, int index, int parent) {
	if (t->index == parent) {
		auto *child = new tree_t(index, t->depth + 1, t, std::vector<tree_t*>());
		t->children.push_back(child);
		return true;
	}
	for (auto *child : t->children) {
		if (add_node(child, index, parent)) {
			return true;
		}
	}
	return false;
}

tree_t *find_node(tree_t *t, int x) {
	if (t->index == x) {
		return t;
	}
	for (auto *child : t->children) {
		auto *n = find_node(child, x);
		if (n) {
			return n;
		}
	}
	return nullptr;
}

int lca(tree_t *t, int x, int y) {
	tree_t *t_x = find_node(t, x);
	tree_t *t_y = find_node(t, y);
	tree_t *deepest = (t_x->depth > t_y->depth) ? t_x : t_y;
	tree_t *other = (t_x->depth > t_y->depth) ? t_y: t_x;
	std::set<int> parents;
	tree_t *parent = deepest->parent;
	while (parent != nullptr) {
		if (parent == other) {
			return other->index;
		}
		parents.emplace(parent->index);
		parent = parent->parent;
	}
	parent = other->parent;
	auto ancestor = parents.find(parent->index);
	while (ancestor == parents.end()) {
		parent = parent->parent;
		ancestor = parents.find(parent->index);
	}
	return *ancestor;
}


int lca_multiple(tree_t *t, const std::vector<int>& parents) {
	int r = lca(t, parents.at(0), parents.at(1));
	for (size_t i = 2; i < parents.size(); i++) {
		r = lca(t, r, parents.at(i));
	}
	return r;
}

void print_tree(tree_t *t) {
	for (int i = 0; i < t->depth; i++) {
		std::cout << "\t";
	}
	std::cout << t->index << ":";
	for (auto *c : t->children) {
		std::cout << " " << c->index;
	}
	std::cout << std::endl;
	for (auto *c : t->children) {
		print_tree(c);
	}
}


// add_virtual_node(transposed);
// auto topological_sort = topological_ordering(transposed);
// auto *lca_tree = new tree_t(topological_sort.front(), 0, nullptr, std::vector<tree_t *>());
// for (auto& i : topological_sort) {
// 	std::cout << i << " ";
// }
// std::cout << std::endl;
// topological_sort.pop_front();
// for (auto& index : topological_sort) {
// 	std::vector<int> parents;
// 	std::cout << index << std::endl;
// 	if (is_tree_node(transposed, index, parents)) {
// 		std::cout << "tree node" << std::endl;
// 		add_node(lca_tree, index, parents.at(0));
// 	} else {
// 		std::cout << "not tree node " << parents.size() << std::endl;
// 		int parent = lca_multiple(lca_tree, parents);
// 		add_node(lca_tree, index, parent);
// 	}
// }
// print_tree(lca_tree);
