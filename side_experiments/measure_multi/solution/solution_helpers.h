#ifndef SOLUTION_HELPERS_H
#define SOLUTION_HELPERS_H

#include <utility>
#include <vector>

#include "action.h"
#include "input.h"
#include "run_helper.h"

class AbstractNode;
class Problem;
class Scope;
class ScopeHistory;
class ScopeNode;
class Solution;

void create_experiment(std::vector<AbstractNode*>& experiment_starts,
					   std::vector<AbstractNode*>& experiment_exit_next_nodes,
					   std::vector<AbstractExperiment*>& experiments,
					   int& curr_experiment_index,
					   int improvement_iter);
void set_experiment_nodes(std::vector<AbstractNode*>& experiment_starts,
						  std::vector<AbstractNode*>& experiment_exit_next_nodes,
						  std::vector<AbstractExperiment*>& experiments);

void gather_possible_helper(ScopeHistory* scope_history,
							std::vector<Scope*>& scope_context,
							std::vector<int>& node_context,
							int& node_count,
							Input& new_input);
void gather_possible_factor_helper(ScopeHistory* scope_history,
								   std::pair<int,int>& new_factor);
void fetch_factor_helper(ScopeHistory* scope_history,
						 std::pair<int,int> factor,
						 double& val);
void fetch_input_helper(ScopeHistory* scope_history,
						Input& input,
						int l_index,
						double& obs);
void fetch_input_helper(ScopeHistory* scope_history,
						Input& input,
						int l_index,
						bool& hit,
						double& obs);

void update_scores(ScopeHistory* scope_history,
				   double target_val);

void clean_scope(Scope* scope);

#endif /* SOLUTION_HELPERS_H */