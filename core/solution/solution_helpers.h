#ifndef SOLUTION_HELPERS_H
#define SOLUTION_HELPERS_H

#include <fstream>
#include <set>
#include <utility>
#include <vector>

#include "context_layer.h"
#include "run_helper.h"

class AbstractNode;
class ActionNode;
class Experiment;
class Scope;
class ScopeHistory;
class ScopeNode;

void create_experiment(ScopeHistory* root_history);
Experiment* create_experiment(std::ifstream& input_file);

ScopeNode* create_existing(Scope* parent_scope,
						   RunHelper& run_helper);
/**
 * - create one scope per run
 */

void gather_possible_helper(std::vector<Scope*>& scope_context,
							std::vector<AbstractNode*>& node_context,
							std::vector<std::vector<Scope*>>& possible_scope_contexts,
							std::vector<std::vector<AbstractNode*>>& possible_node_contexts,
							ScopeHistory* scope_history);

void input_vals_helper(int curr_depth,
					   int max_depth,
					   std::vector<Scope*>& scope_context,
					   std::vector<AbstractNode*>& node_context,
					   std::vector<double>& input_vals,
					   ScopeHistory* scope_history);

#endif /* SOLUTION_HELPERS_H */