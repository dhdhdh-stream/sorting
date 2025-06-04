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

const int PASS_THROUGH_EXPERIMENT_STATE_MEASURE_1_PERCENT = 0;
const int PASS_THROUGH_EXPERIMENT_STATE_MEASURE_5_PERCENT = 1;
const int PASS_THROUGH_EXPERIMENT_STATE_MEASURE_10_PERCENT = 2;
const int PASS_THROUGH_EXPERIMENT_STATE_MEASURE_25_PERCENT = 3;
const int PASS_THROUGH_EXPERIMENT_STATE_MEASURE_50_PERCENT = 4;

class PassThroughExperimentHistory;
class PassThroughExperiment : public AbstractExperiment {
public:
	int state;
	int state_iter;

	std::vector<int> step_types;
	std::vector<Action> actions;
	std::vector<Scope*> scopes;
	AbstractNode* exit_next_node;

	double existing_sum_score;
	int existing_count;
	double new_sum_score;
	int new_count;

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

	void clean_inputs(Scope* scope,
					  int node_id);
	void clean_inputs(Scope* scope);
	void replace_factor(Scope* scope,
						int original_node_id,
						int original_factor_index,
						int new_node_id,
						int new_factor_index);
	void replace_obs_node(Scope* scope,
						  int original_node_id,
						  int new_node_id);
	void replace_scope(Scope* original_scope,
					   Scope* new_scope,
					   int new_scope_node_id);

	void clean();
	void add();
};

class PassThroughExperimentHistory : public AbstractExperimentHistory {
public:
	PassThroughExperimentHistory(PassThroughExperiment* experiment);
};

#endif /* PASS_THROUGH_EXPERIMENT_H */