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

void gather_dependencies_helper(ScopeHistory* scope_history,
								std::vector<int>& curr_context,
								int& count,
								std::vector<int>& dependency);
void fetch_dependency_helper(ScopeHistory* scope_history,
							 std::vector<AbstractNode*>& dependencies,
							 int l_index,
							 bool& is_hit,
							 std::vector<double>& state,
							 std::vector<double>& obs);

void update_helper(ScopeHistory* scope_history);
void update_helper(double target_val,
				   SolutionWrapper* wrapper);

double measure_helper(SolutionWrapper* wrapper);

#endif /* SOLUTION_HELPERS_H */