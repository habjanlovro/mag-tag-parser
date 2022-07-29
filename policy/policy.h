#ifndef _POLICY_POLICY_H_
#define _POLICY_POLICY_H_

#include <string>
#include <set>
#include <map>
#include <vector>
#include <iostream>
#include <memory>


class topology_t {
	public:
		virtual ~topology_t() {}
		std::string& get_name() {
			return name;
		}
		virtual void print() {
			std::cout << name << std::endl;
		}
		virtual std::string fullname(const std::string& tag) {
			return name + "." + tag;
		}
	protected:
		std::string name;
};


class topology_linear_t : public topology_t {
	public:
		topology_linear_t(const std::string& n) {
			name = n;
			tags = std::vector<std::string>();
		}
		topology_linear_t(const std::string& n, const std::vector<std::string>& ts)
				: tags(ts) {
			name = n;
		}
		void add_tag(const std::string& tag) {
			std::string fullname = name + "." + tag;
			tags.push_back(fullname);
		}

		std::vector<std::string>& get_tags() {
			return tags;
		}

		void print() {
			std::cout << "Topology '" << name << "'" << std::endl;
			std::cout << "\t";
			for (auto& t : tags) {
				std::cout << t << ",";
			}
			std::cout << std::endl;
		}
	private:
		std::vector<std::string> tags;
};

class topology_basic_t : public topology_t {
	public:
		topology_basic_t(const std::string& n);
		topology_basic_t(const std::string& n, const std::set<std::string>& vertices);
		topology_basic_t(topology_linear_t& t);
		void add_edge(const std::string& source, const std::string& end);
		size_t size() {
			return mvertices.size();
		}
		std::vector<std::vector<uint8_t>>& matrix() {
			return mvertices;
		}
		std::map<std::string, int>& index_mapping() {
			return toindex;
		}
		void disjoint_union(
			const std::shared_ptr<topology_basic_t>& t1,
			const std::shared_ptr<topology_basic_t>& t2);
		void carthesian_product(
			const std::shared_ptr<topology_basic_t>& t1,
			const std::shared_ptr<topology_basic_t>& t2);
		void print();
		void set_name_prefix(const std::string& prefix);
	private:
		std::vector<std::vector<uint8_t>> mvertices;
		std::map<std::string, int> toindex;
};

class policy_t {
	public:
		policy_t() {}
		policy_t(const char *file_path);
		bool contains_tag(const std::string& tag) const {
			return tags.find(tag) != tags.end();
		}
	private:
		std::map<std::string, std::shared_ptr<topology_t>> topologies;
		std::set<std::string> tags;
		std::shared_ptr<topology_basic_t> topology;
};


#endif
