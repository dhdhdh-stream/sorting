#ifndef SOLUTION_NODE_UTILITIES_H
#define SOLUTION_NODE_UTILITIES_H

#include <vector>

#include "solution_node.h"

void find_scope_end(SolutionNode* inclusive_start,
					SolutionNode*& inclusive_end,
					SolutionNode*& non_inclusive_end);

void find_potential_jumps(SolutionNode* inclusive_start,
						  std::vector<SolutionNode*>& potential_inclusive_jump_ends,
						  std::vector<SolutionNode*>& potential_non_inclusive_jump_ends);

void find_potential_loops(SolutionNode* inclusive_end,
						  std::vector<SolutionNode*>& potential_non_inclusive_loop_starts,
						  std::vector<SolutionNode*>& potential_inclusive_loop_starts);

#endif /* SOLUTION_NODE_UTILITIES_H */