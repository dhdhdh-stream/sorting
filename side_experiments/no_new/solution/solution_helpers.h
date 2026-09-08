#ifndef SOLUTION_HELPERS_H
#define SOLUTION_HELPERS_H

#include <map>
#include <set>
#include <utility>
#include <vector>

#include <Eigen/Dense>

class AbstractExperiment;
class AbstractNode;
class BranchNode;
class Problem;
class Scope;
class ScopeHistory;
class ScopeNode;
class Solution;
class SolutionWrapper;

void create_predict_experiment(ScopeHistory* scope_history,
							   SolutionWrapper* wrapper);
void create_explore(ScopeHistory* scope_history,
					SolutionWrapper* wrapper);

void update_helper(double target_val,
				   SolutionWrapper* wrapper);

void train_helper(SolutionWrapper* wrapper);

double predict_helper(AbstractNode* starting_next_node,
					  Eigen::VectorXf& starting_state,
					  Scope* scope_context);

double measure_helper(SolutionWrapper* wrapper);

#endif /* SOLUTION_HELPERS_H */