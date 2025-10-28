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

void create_branch_experiment(ScopeHistory* scope_history,
							  SolutionWrapper* wrapper);

AbstractNode* fetch_path_end(AbstractNode* node_context,
							 bool is_branch);

Scope* create_new_scope(Scope* scope_context);

void clean_scope(Scope* scope,
				 SolutionWrapper* wrapper);

#endif /* SOLUTION_HELPERS_H */