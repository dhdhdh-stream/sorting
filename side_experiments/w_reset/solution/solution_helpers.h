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

void update_helper(ScopeHistory* scope_history,
				   double target_val,
				   std::set<BranchNode*>& hit_original,
				   std::set<BranchNode*>& hit_branch);
void update_helper(std::set<BranchNode*>& hit_original,
				   std::set<BranchNode*>& hit_branch);

#endif /* SOLUTION_HELPERS_H */