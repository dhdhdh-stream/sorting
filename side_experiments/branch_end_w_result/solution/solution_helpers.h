#ifndef SOLUTION_HELPERS_H
#define SOLUTION_HELPERS_H

#include <map>
#include <utility>
#include <vector>

#include "input.h"

class AbstractNode;
class BranchExperiment;
class Network;
class NewScopeOverallExperiment;
class Problem;
class Scope;
class ScopeHistory;
class ScopeNode;
class Solution;
class SolutionWrapper;

bool is_new_scope_iter(SolutionWrapper* wrapper);

void create_branch_experiment(ScopeHistory* scope_history,
							  SolutionWrapper* wrapper);
void create_new_scope_overall_experiment(ScopeHistory* scope_history,
										 SolutionWrapper* wrapper);
void create_new_scope_experiment(ScopeHistory* scope_history,
								 SolutionWrapper* wrapper);
bool still_instances_possible(NewScopeOverallExperiment* experiment);

AbstractNode* fetch_path_end(AbstractNode* node_context);

void fetch_input_helper(ScopeHistory* scope_history,
						Input& input,
						int l_index,
						double& obs,
						bool& is_on);
void fetch_input_helper(ScopeHistory* scope_history,
						Input& input,
						int l_index,
						double& obs,
						bool& is_on,
						int& num_actions_snapshot);

Scope* create_new_scope(Scope* scope_context,
						SolutionWrapper* wrapper,
						bool include_external);
void recursive_add_child(Scope* curr_parent,
						 SolutionWrapper* wrapper,
						 Scope* new_scope);

void clean_scope(Scope* scope,
				 SolutionWrapper* wrapper);

void update_scores(std::vector<ScopeHistory*>& scope_histories,
				   std::vector<double>& target_val_histories,
				   SolutionWrapper* wrapper);

#endif /* SOLUTION_HELPERS_H */