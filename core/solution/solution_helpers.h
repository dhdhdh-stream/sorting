#ifndef SOLUTION_HELPERS_H
#define SOLUTION_HELPERS_H

#include <utility>
#include <vector>

#include "context_layer.h"
#include "run_helper.h"

class AbstractNode;
class BranchNode;
class Scope;
class ScopeHistory;
class ScopeNode;

void create_experiment(ScopeHistory* root_history);

ScopeNode* reuse_existing();
ScopeNode* create_scope(Scope* parent_scope,
						RunHelper& run_helper);
ScopeNode* create_repeat(std::vector<ContextLayer>& context,
						 int explore_context_depth);

void gather_possible_exits(std::vector<std::pair<int,AbstractNode*>>& possible_exits,
						   std::vector<Scope*>& experiment_scope_context,
						   std::vector<AbstractNode*>& experiment_node_context,
						   bool experiment_is_branch);
void parent_pass_through_gather_possible_exits(
	std::vector<std::pair<int,AbstractNode*>>& possible_exits,
	std::vector<Scope*>& scope_context,
	std::vector<AbstractNode*>& node_context,
	int parent_exit_depth,
	AbstractNode* parent_exit_node);

void gather_possible_helper(std::vector<Scope*>& scope_context,
							std::vector<AbstractNode*>& node_context,
							std::vector<std::vector<Scope*>>& possible_scope_contexts,
							std::vector<std::vector<AbstractNode*>>& possible_node_contexts,
							ScopeHistory* scope_history);

void create_gather(std::vector<Scope*>& new_gather_scope_context,
				   std::vector<AbstractNode*>& new_gather_node_context,
				   bool& new_gather_is_branch,
				   int& new_gather_exit_depth,
				   AbstractNode*& new_gather_exit_node,
				   ScopeHistory* scope_history,
				   BranchNode* filter_branch_node);

/**
 * TODO:
 * - instead of always backtracking, add caching?
 */
void input_vals_helper(std::vector<Scope*>& scope_context,
					   std::vector<AbstractNode*>& node_context,
					   std::vector<double>& input_vals,
					   ScopeHistory* scope_history);

#endif /* SOLUTION_HELPERS_H */