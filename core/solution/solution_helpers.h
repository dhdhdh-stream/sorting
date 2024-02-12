#ifndef SOLUTION_HELPERS_H
#define SOLUTION_HELPERS_H

#include <utility>
#include <vector>

#include "context_layer.h"

class AbstractNode;
class Problem;
class Scope;
class ScopeHistory;
class ScopeNode;

void create_experiment(ScopeHistory* root_history);

ScopeNode* reuse_existing(Problem* problem);
ScopeNode* create_scope(Scope* parent_scope);
ScopeNode* create_repeat(std::vector<ContextLayer>& context,
						 int explore_context_depth);

void gather_possible_exits(std::vector<std::pair<int,AbstractNode*>>& possible_exits,
						   std::vector<ContextLayer>& context,
						   std::vector<Scope*>& scope_context,
						   std::vector<AbstractNode*>& node_context);
void parent_pass_through_gather_possible_exits(
	std::vector<std::pair<int,AbstractNode*>>& possible_exits,
	std::vector<ContextLayer>& context,
	std::vector<Scope*>& scope_context,
	std::vector<AbstractNode*>& node_context,
	int parent_exit_depth,
	AbstractNode* parent_exit_node);

void gather_possible_helper(std::vector<Scope*>& scope_context,
							std::vector<AbstractNode*>& node_context,
							std::vector<std::vector<Scope*>>& possible_scope_contexts,
							std::vector<std::vector<AbstractNode*>>& possible_node_contexts,
							ScopeHistory* scope_history);

/**
 * TODO:
 * - instead of always backtracking, add caching?
 */
void input_vals_helper(std::vector<Scope*>& scope_context,
					   std::vector<AbstractNode*>& node_context,
					   std::vector<double>& input_vals,
					   ScopeHistory* scope_history);

#endif /* SOLUTION_HELPERS_H */