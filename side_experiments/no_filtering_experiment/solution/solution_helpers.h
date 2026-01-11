#ifndef SOLUTION_HELPERS_H
#define SOLUTION_HELPERS_H

#include <map>
#include <utility>
#include <vector>

class AbstractExperiment;
class AbstractNode;
class Network;
class Problem;
class ProblemType;
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

void clean_scope(Scope* scope);

#endif /* SOLUTION_HELPERS_H */