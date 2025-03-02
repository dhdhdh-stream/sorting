#ifndef PASS_THROUGH_EXPERIMENT_H
#define PASS_THROUGH_EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"
#include "action.h"
#include "run_helper.h"

class AbstractNode;
class Problem;
class Scope;
class Solution;

class PassThroughExperimentHistory;
class PassThroughExperiment : public AbstractExperiment {
public:
	int state_iter;
	int sub_state_iter;
	int explore_iter;

	double curr_sum_score;
	std::vector<int> curr_step_types;
	std::vector<Action> curr_actions;
	std::vector<Scope*> curr_scopes;
	AbstractNode* curr_exit_next_node;

	double best_score;
	std::vector<int> best_step_types;
	std::vector<Action> best_actions;
	std::vector<Scope*> best_scopes;
	AbstractNode* best_exit_next_node;

	PassThroughExperiment(Scope* scope_context,
						  AbstractNode* node_context,
						  bool is_branch);
	void decrement(AbstractNode* experiment_node);

	void activate(AbstractNode* experiment_node,
				  bool is_branch,
				  AbstractNode*& curr_node,
				  Problem* problem,
				  RunHelper& run_helper,
				  ScopeHistory* scope_history);
	void backprop(double target_val,
				  RunHelper& run_helper);

	void explore_activate(AbstractNode*& curr_node,
						  Problem* problem,
						  RunHelper& run_helper);
	void explore_backprop(double target_val,
						  RunHelper& run_helper);

	void finalize(Solution* duplicate);
};

class PassThroughExperimentHistory : public AbstractExperimentHistory {
public:
	PassThroughExperimentHistory(PassThroughExperiment* experiment);
};

#endif /* PASS_THROUGH_EXPERIMENT_H */