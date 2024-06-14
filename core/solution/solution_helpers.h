#ifndef SOLUTION_HELPERS_H
#define SOLUTION_HELPERS_H

#include <fstream>
#include <set>
#include <utility>
#include <vector>

#include "context_layer.h"
#include "run_helper.h"

class AbstractNode;
class InfoScope;
class InfoScopeNode;
class Problem;
class Scope;
class ScopeHistory;
class ScopeNode;
class Solution;

void create_experiment(RunHelper& run_helper);

ScopeNode* create_existing();

Scope* create_new_info_subscope();

void gather_possible_helper(std::vector<AbstractNode*>& possible_node_contexts,
							std::vector<int>& possible_obs_indexes,
							ScopeHistory* scope_history);

void create_new_action(AbstractNode*& starting_node,
					   std::set<AbstractNode*>& included_nodes);

void clean_scope(Scope* scope);

#endif /* SOLUTION_HELPERS_H */