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

bool check_can_protect(AbstractNode* starting_node,
					   AbstractNode* ending_node);

void create_experiment(ScopeHistory* scope_history,
					   SolutionWrapper* wrapper);

void create_new_scope(AbstractNode* potential_start_node,
					  AbstractNode* potential_end_node,
					  Scope*& new_scope);
void create_new_scope(Scope* scope_context,
					  SolutionWrapper* wrapper,
					  Scope*& new_scope,
					  Scope*& parent_scope);
void outer_create_new_scope(Scope* scope_context,
							SolutionWrapper* wrapper,
							Scope*& new_scope,
							Scope*& parent_scope);
void recursive_add_child(Scope* curr_parent,
						 SolutionWrapper* wrapper,
						 Scope* new_scope);

void clean_scope(Scope* scope);

double result_helper(SolutionWrapper* wrapper);

#endif /* SOLUTION_HELPERS_H */