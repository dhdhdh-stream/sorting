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

const int PASS_THROUGH_EXPERIMENT_STATE_INITIAL = 0;
const int PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST = 1;
const int PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND = 2;

class PassThroughExperimentHistory;
class PassThroughExperiment : public AbstractExperiment {
public:
	int state;
	int state_iter;
	int explore_iter;

	std::vector<int> step_types;
	std::vector<Action> actions;
	std::vector<Scope*> scopes;
	AbstractNode* exit_next_node;

	double sum_score;

	PassThroughExperiment(Scope* scope_context,
						  AbstractNode* node_context,
						  bool is_branch);
	void decrement(AbstractNode* experiment_node);

	void result_activate(AbstractNode* experiment_node,
						 bool is_branch,
						 AbstractNode*& curr_node,
						 Problem* problem,
						 RunHelper& run_helper,
						 ScopeHistory* scope_history);
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

	void clean();
	void add();
};

class PassThroughExperimentHistory : public AbstractExperimentHistory {
public:
	PassThroughExperimentHistory(PassThroughExperiment* experiment);
};

#endif /* PASS_THROUGH_EXPERIMENT_H */