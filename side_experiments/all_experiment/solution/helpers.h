#ifndef HELPERS_H
#define HELPERS_H

#include <map>
#include <utility>
#include <vector>

class AbstractNode;
class BranchExperiment;
class Network;
class ObsNode;
class Problem;
class Scope;
class ScopeHistory;
class ScopeNode;
class Solution;
class SolutionWrapper;

void create_experiment(ObsNode* obs_node,
					   SolutionWrapper* wrapper);
void create_experiments(SolutionWrapper* wrapper);

Scope* create_new_scope(Scope* scope_context);

void clean_scope(Scope* scope,
				 SolutionWrapper* wrapper);

// temp
double result_helper(SolutionWrapper* wrapper);

void iter_update_vals_helper(ScopeHistory* scope_history,
							 double target_val);
void update_vals(SolutionWrapper* wrapper);
void iter_update_damage_helper(ScopeHistory* scope_history,
							   double target_val);
void update_damage(SolutionWrapper* wrapper);

#endif /* HELPERS_H */