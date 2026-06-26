#ifndef SOLUTION_HELPERS_H
#define SOLUTION_HELPERS_H

#include <map>
#include <set>
#include <utility>
#include <vector>

class AbstractNode;
class BranchExperiment;
class BranchNode;
class Network;
class Problem;
class Scope;
class ScopeHistory;
class ScopeNode;
class Solution;
class SolutionWrapper;

void create_experiment(ScopeHistory* scope_history,
					   SolutionWrapper* wrapper);

void clean_scope(Scope* scope);

void update_helper(double target_val,
				   SolutionWrapper* wrapper);

double get_existing_result(SolutionWrapper* wrapper);

#endif /* SOLUTION_HELPERS_H */