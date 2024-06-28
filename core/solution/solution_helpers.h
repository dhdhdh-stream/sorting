#ifndef SOLUTION_HELPERS_H
#define SOLUTION_HELPERS_H

#include <fstream>
#include <set>
#include <utility>
#include <vector>

#include "context_layer.h"
#include "run_helper.h"

class AbstractNode;
class BranchNode;
class InfoScope;
class InfoScopeNode;
class Problem;
class Scope;
class ScopeHistory;
class ScopeNode;
class Solution;

void create_experiment(RunHelper& run_helper);

ScopeNode* create_existing(Scope* parent_scope);
InfoScope* get_existing_info_scope(Scope* parent_scope);

InfoScope* create_new_info_scope();

void gather_possible_helper(std::vector<AbstractScope*>& scope_context,
							std::vector<AbstractNode*>& node_context,
							std::vector<std::vector<AbstractScope*>>& possible_scope_contexts,
							std::vector<std::vector<AbstractNode*>>& possible_node_contexts,
							std::vector<int>& possible_obs_indexes,
							AbstractScopeHistory* scope_history);
void gather_new_action_included_nodes(AbstractNode* starting_node,
									  std::vector<AbstractNode*>& included_nodes);

void create_new_action(AbstractNode*& starting_node,
					   std::set<AbstractNode*>& included_nodes);

void clean_scope(Scope* scope,
				 Solution* parent_solution);
void clean_info_scope(InfoScope* scope);

void add_branch_node_familiarity(BranchNode* branch_node,
								 std::vector<ScopeHistory*>& scope_histories);
void add_eval_familiarity(Scope* scope,
						  std::vector<ScopeHistory*>& scope_histories);
void measure_familiarity(Scope* scope,
						 std::vector<ScopeHistory*>& scope_histories);

#endif /* SOLUTION_HELPERS_H */