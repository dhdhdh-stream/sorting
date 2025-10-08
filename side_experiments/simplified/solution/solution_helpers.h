#ifndef SOLUTION_HELPERS_H
#define SOLUTION_HELPERS_H

#include <map>
#include <utility>
#include <vector>

#include "input.h"

class AbstractExperiment;
class AbstractNode;
class Network;
class Problem;
class Scope;
class ScopeHistory;
class ScopeNode;
class Solution;
class SolutionWrapper;

void create_experiment(SolutionWrapper* wrapper,
					   AbstractExperiment*& curr_experiment);

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

double get_experiment_impact(AbstractExperiment* experiment);

void clean_scope(Scope* scope,
				 SolutionWrapper* wrapper);

void update_scores(ScopeHistory* scope_history,
				   double target_val,
				   int h_index);

#endif /* SOLUTION_HELPERS_H */