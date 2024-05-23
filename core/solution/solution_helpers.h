#ifndef SOLUTION_HELPERS_H
#define SOLUTION_HELPERS_H

#include <fstream>
#include <set>
#include <utility>
#include <vector>

#include "context_layer.h"
#include "run_helper.h"

class AbstractNode;
class Eval;
class InfoScope;
class InfoScopeNode;
class Problem;
class Scope;
class ScopeHistory;
class ScopeNode;

void create_experiment(ScopeHistory* root_history);

ScopeNode* create_existing();
InfoScope* get_existing_info_scope();
InfoScopeNode* create_existing_info_scope_node();

Scope* create_new_info_scope();

void gather_possible_helper(std::vector<Scope*>& scope_context,
							std::vector<AbstractNode*>& node_context,
							std::vector<std::vector<Scope*>>& possible_scope_contexts,
							std::vector<std::vector<AbstractNode*>>& possible_node_contexts,
							std::vector<int>& possible_obs_indexes,
							ScopeHistory* scope_history);
void gather_eval_possible_helper(std::vector<AbstractNode*>& possible_node_contexts,
								 std::vector<int>& possible_obs_indexes,
								 EvalHistory* eval_history);

void create_new_action(AbstractNode*& starting_node,
					   std::set<AbstractNode*>& included_nodes);

void random_sequence(AbstractNode*& curr_node,
					 Problem* problem,
					 std::vector<ContextLayer>& context,
					 RunHelper& run_helper);

#endif /* SOLUTION_HELPERS_H */