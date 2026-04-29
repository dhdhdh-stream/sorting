#ifndef SOLUTION_HELPERS_H
#define SOLUTION_HELPERS_H

#include <map>
#include <set>
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

void count_eval_helper(ScopeHistory* scope_history,
					   int& node_count,
					   int& eval_count);
void create_experiment(ScopeHistory* scope_history,
					   SolutionWrapper* wrapper);

void gather_all_children_helper(AbstractNode* curr_node,
								std::set<AbstractNode*>& children);

void create_new_scope(AbstractNode* potential_start_node,
					  AbstractNode* potential_end_node,
					  Scope*& new_scope);
Scope* create_new_scope(Scope* scope_context,
						SolutionWrapper* wrapper);
Scope* outer_create_new_scope(SolutionWrapper* wrapper);
void recursive_add_child(Scope* curr_parent,
						 SolutionWrapper* wrapper,
						 Scope* new_scope);

void clean_scope(Scope* scope);

bool simulate_helper(SolutionWrapper* wrapper);

double measure_helper(SolutionWrapper* solution_wrapper);

#endif /* SOLUTION_HELPERS_H */