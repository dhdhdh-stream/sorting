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

void update_helper(ScopeHistory* scope_history,
				   double target_val,
				   std::set<BranchNode*>& hit_original,
				   std::set<BranchNode*>& hit_branch);
void update_helper(std::set<BranchNode*>& hit_original,
				   std::set<BranchNode*>& hit_branch);

double measure_helper(SolutionWrapper* wrapper);

void add_crazy_helper(Scope* scope_context,
					  AbstractNode* node_context,
					  bool is_branch,
					  AbstractNode* exit_next_node,
					  SolutionWrapper* wrapper);

double get_existing_result(SolutionWrapper* wrapper);

#endif /* SOLUTION_HELPERS_H */