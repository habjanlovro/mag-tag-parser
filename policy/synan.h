#ifndef _POLICY_SYNAN_H_
#define _POLICY_SYNAN_H_

#include "lexer.h"

#include <vector>


enum class Nont {
	SOURCE,
	DECLS,
	DECLREST,
	DECL,
	TOPOLOGY,
	TOPOLOGYREST,
	BASIC,
	EDGE,
	EDGEREST,
	LINEAR,
	LINEARREST,
	EXPR,
	SUM,
	SUMREST,
	MUL,
	MULREST,
	ELEM
};


struct dertree_t {
	Nont label;
	std::vector<dertree_t> subtrees;
	std::vector<symbol_t> leaves;
};


dertree_t parse_source(std::vector<symbol_t>& symbols);

#endif