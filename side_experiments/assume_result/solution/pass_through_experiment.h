#ifndef PASS_THROUGH_EXPERIMENT_H
#define PASS_THROUGH_EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"
#include "context_layer.h"
#include "run_helper.h"

class AbstractNode;
class ActionNode;
class Problem;
class ReturnNode;
class Scope;
class ScopeNode;
class Solution;

const int PASS_THROUGH_EXPERIMENT_STATE_EXPLORE = 0;

class PassThroughExperimentHistory;
class PassThroughExperiment : public AbstractExperiment {
public:
	int state_iter;
	int sub_state_iter;
	int explore_iter;

	double existing_average_score;

	double curr_score;
	AbstractNode* curr_previous_location;
	std::vector<int> curr_step_types;
	std::vector<ActionNode*> curr_actions;
	std::vector<ScopeNode*> curr_scopes;
	std::vector<ReturnNode*> curr_returns;
	AbstractNode* curr_exit_next_node;

	double best_score;
	AbstractNode* best_previous_location;
	std::vector<int> best_step_types;
	std::vector<ActionNode*> best_actions;
	std::vector<ScopeNode*> best_scopes;
	std::vector<ReturnNode*> best_returns;
	AbstractNode* best_exit_next_node;

	ActionNode* ending_node;

	std::vector<double> target_val_histories;

	PassThroughExperiment(Scope* scope_context,
						  AbstractNode* node_context,
						  bool is_branch);
	~PassThroughExperiment();
	void decrement(AbstractNode* experiment_node);

	bool activate(AbstractNode* experiment_node,
				  bool is_branch,
				  AbstractNode*& curr_node,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  RunHelper& run_helper);
	void backprop(double target_val,
				  RunHelper& run_helper);

	void explore_activate(AbstractNode*& curr_node,
						  Problem* problem,
						  std::vector<ContextLayer>& context,
						  RunHelper& run_helper,
						  PassThroughExperimentHistory* history);
	void explore_backprop(double target_val,
						  RunHelper& run_helper);

	void finalize(Solution* duplicate);
	void new_branch(Solution* duplicate);
	void new_pass_through(Solution* duplicate);
};

class PassThroughExperimentHistory : public AbstractExperimentHistory {
public:
	int instance_count;

	PassThroughExperimentHistory(PassThroughExperiment* experiment);
};

#endif /* PASS_THROUGH_EXPERIMENT_H */