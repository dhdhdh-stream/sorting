#ifndef SOLUTION_NODE_UTILITIES_H
#define SOLUTION_NODE_UTILITIES_H

#include <vector>

#include "solution_node.h"

void new_random_scope(SolutionNode* explore_node,
					  int& parent_jump_scope_start_non_inclusive_index,
					  int& parent_jump_end_non_inclusive_index);

void new_random_path(std::vector<SolutionNode*>& explore_path,
					 bool can_be_empty);

void get_existing_path(SolutionNode* explore_node,
					   std::vector<SolutionNode*>& existing_path);
void get_replacement_path(SolutionNode* explore_node,
						  std::vector<SolutionNode*>& replacement_path);

bool is_after_explore(std::vector<SolutionNode*>& current_scope,
					  std::vector<int>& current_scope_states,
					  std::vector<int>& current_scope_locations,
					  std::vector<SolutionNode*>& explore_scope,
					  std::vector<int>& explore_scope_states,
					  std::vector<int>& explore_scope_locations,
					  int explore_parent_jump_end_non_inclusive_index);

SolutionNode* get_jump_scope_start(SolutionNode* explore_node);
SolutionNode* get_jump_end(IterExplore* iter_explore,
						   SolutionNode* explore_node);

#endif /* SOLUTION_NODE_UTILITIES_H */