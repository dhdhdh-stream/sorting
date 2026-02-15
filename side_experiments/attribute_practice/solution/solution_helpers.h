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

void signal_add_existing_sample(ScopeHistory* scope_history,
								double target_val);
void signal_add_explore_sample(ScopeHistory* scope_history,
							   double target_val);

void clean_scope(Scope* scope);

void get_existing_result_init(SolutionWrapper* wrapper);
void get_existing_result(SolutionWrapper* wrapper);

#endif /* SOLUTION_HELPERS_H */