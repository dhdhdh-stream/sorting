#ifndef SOLUTION_NODE_UTILITIES_H
#define SOLUTION_NODE_UTILITIES_H

#include <vector>

#include "solution_node.h"

void new_random_scope(SolutionNode* explore_node,
					  int& parent_jump_scope_start_non_inclusive_index,
					  int& parent_jump_end_non_inclusive_index);

void new_random_path(std::vector<SolutionNode*>& explore_path,
					 bool can_be_empty);

SolutionNode* get_jump_end(SolutionNode* explore_node);

#endif /* SOLUTION_NODE_UTILITIES_H */