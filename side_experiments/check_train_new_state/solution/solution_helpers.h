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
								std::vector<int>& curr_index,
								int& count,
								std::vector<int>& dependency,
								std::vector<int>& index);
void fetch_dependency_helper(ScopeHistory* scope_history,
							 std::vector<int>& dependency,
							 int l_index,
							 bool& is_hit,
							 std::vector<double>& obs);

void update_helper(ScopeHistory* scope_history,
				   double target_val,
				   SolutionWrapper* wrapper,
				   std::set<BranchNode*>& hit_original,
				   std::set<BranchNode*>& hit_branch);
void update_helper(std::set<BranchNode*>& hit_original,
				   std::set<BranchNode*>& hit_branch,
				   SolutionWrapper* wrapper);

double measure_helper(SolutionWrapper* wrapper);

#endif /* SOLUTION_HELPERS_H */