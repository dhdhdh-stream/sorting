#ifndef SOLUTION_HELPERS_H
#define SOLUTION_HELPERS_H

#include <fstream>
#include <set>
#include <utility>
#include <vector>

#include "context_layer.h"
#include "run_helper.h"

class AbstractExperiment;
class AbstractNode;
class ActionNode;
class Scope;
class ScopeHistory;
class ScopeNode;

void create_experiment(ScopeHistory* root_history);
AbstractExperiment* create_experiment(std::ifstream& input_file);

ScopeNode* create_existing();

/**
 * - don't use actual run as might involve nodes not always reachable
 */
void gather_possible_exits(std::vector<std::pair<int,AbstractNode*>>& possible_exits,
						   std::vector<Scope*>& experiment_scope_context,
						   std::vector<AbstractNode*>& experiment_node_context,
						   bool experiment_is_branch);

void gather_possible_helper(std::vector<Scope*>& scope_context,
							std::vector<AbstractNode*>& node_context,
							std::vector<std::vector<Scope*>>& possible_scope_contexts,
							std::vector<std::vector<AbstractNode*>>& possible_node_contexts,
							std::vector<int>& possible_obs_indexes,
							ScopeHistory* scope_history);

void input_vals_helper(int curr_depth,
					   int max_depth,
					   std::vector<Scope*>& scope_context,
					   std::vector<AbstractNode*>& node_context,
					   std::vector<double>& input_vals,
					   ScopeHistory* scope_history);

#endif /* SOLUTION_HELPERS_H */