#ifndef _POLICY_LCA_H_
#define _POLICY_LCA_H_

#include <vector>
#include <stdint.h>

#define TAG_INVALID ((uint8_t) ((1 << 8) - 1))


std::vector<std::vector<uint8_t>> compute_lca(const std::vector<std::vector<uint8_t>>& m);

#endif
