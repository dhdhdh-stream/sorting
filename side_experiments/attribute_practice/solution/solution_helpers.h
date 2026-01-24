#ifndef SOLUTION_HELPERS_H
#define SOLUTION_HELPERS_H

#include <map>
#include <utility>
#include <vector>

class AbstractNode;
class BranchExperiment;
class Network;
class Problem;
class Scope;
class ScopeHistory;
class ScopeNode;
class Solution;
class SolutionWrapper;

void create_experiment(ScopeHistory* scope_history,
					   SolutionWrapper* wrapper);

Scope* create_new_scope(Scope* scope_context);
void recursive_add_child(Scope* curr_parent,
						 SolutionWrapper* wrapper,
						 Scope* new_scope);

void update_attribute(ScopeHistory* scope_history,
					  double target_val);

void eval_curr_tunnel_helper(ScopeHistory* scope_history,
							 SolutionWrapper* wrapper,
							 double& sum_vals);

void clean_scope(Scope* scope);

#endif /* SOLUTION_HELPERS_H */