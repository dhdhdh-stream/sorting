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

void create_new_scope(Scope* scope_context,
					  SolutionWrapper* wrapper,
					  Scope*& new_scope,
					  Scope*& parent_scope);
void recursive_add_child(Scope* curr_parent,
						 SolutionWrapper* wrapper,
						 Scope* new_scope);

void clean_scope(Scope* scope);

double get_existing_result(SolutionWrapper* wrapper);

#endif /* SOLUTION_HELPERS_H */