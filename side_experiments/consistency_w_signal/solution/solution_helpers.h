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

double calc_signal(std::vector<ScopeHistory*>& post_scope_histories,
				   double target_val,
				   SolutionWrapper* wrapper);

void add_existing_samples_helper(ScopeHistory* scope_history);
void add_explore_samples_helper(ScopeHistory* scope_history,
								std::vector<ScopeHistory*>& stack_trace,
								double target_val,
								SolutionWrapper* wrapper);

void clean_scope(Scope* scope);

#endif /* SOLUTION_HELPERS_H */