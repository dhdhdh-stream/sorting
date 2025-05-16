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
	int explore_iter;

	int exceed_max_length;

	bool needs_init;

	std::vector<int> step_types;
	std::vector<Action> actions;
	std::vector<Scope*> scopes;
	AbstractNode* exit_next_node;

	std::vector<double> existing_target_vals;
	std::vector<std::vector<std::pair<int,bool>>> existing_influence_indexes;
	std::vector<double> new_target_vals;
	std::vector<std::vector<std::pair<int,bool>>> new_influence_indexes;

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
				  bool is_return,
				  RunHelper& run_helper);

	void explore_activate(AbstractNode*& curr_node,
						  Problem* problem,
						  RunHelper& run_helper);
	void explore_backprop(double target_val,
						  bool is_return,
						  RunHelper& run_helper);

	void clean();
	void add();

private:
	void calc_improve_helper(bool& is_success,
							 double& curr_improvement);
};

class PassThroughExperimentHistory : public AbstractExperimentHistory {
public:
	PassThroughExperimentHistory(PassThroughExperiment* experiment);
};

#endif /* PASS_THROUGH_EXPERIMENT_H */