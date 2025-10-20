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

void create_branch_experiment(SolutionWrapper* wrapper);
void create_new_scope_overall_experiment(SolutionWrapper* wrapper);
void create_new_scope_experiment(SolutionWrapper* wrapper);
bool still_instances_possible(NewScopeOverallExperiment* experiment);

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

Scope* create_new_scope(Scope* scope_context);

double get_experiment_impact(BranchExperiment* experiment);
double get_experiment_impact(NewScopeOverallExperiment* experiment);

void clean_scope(Scope* scope,
				 SolutionWrapper* wrapper);

void update_scores(ScopeHistory* scope_history,
				   double target_val,
				   int h_index);

#endif /* SOLUTION_HELPERS_H */