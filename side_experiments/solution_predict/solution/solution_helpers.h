#ifndef SOLUTION_HELPERS_H
#define SOLUTION_HELPERS_H

#include <map>
#include <set>
#include <utility>
#include <vector>

#include <Eigen/Dense>

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
					   int diversity_index,
					   SolutionWrapper* wrapper);

double predict_helper(Eigen::VectorXf& state,
					  AbstractNode* exit_next_node,
					  std::vector<AbstractNode*>& node_context,
					  SolutionWrapper* wrapper);

void update_helper(SolutionWrapper* wrapper,
				   double target_val);

void train_existing_helper(SolutionWrapper* wrapper);
void train_explore_helper(SolutionWrapper* wrapper);

#endif /* SOLUTION_HELPERS_H */