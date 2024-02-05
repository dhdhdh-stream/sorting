#ifndef PASS_THROUGH_EXPERIMENT_H
#define PASS_THROUGH_EXPERIMENT_H

const int PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING_SCORE = 0;
const int PASS_THROUGH_EXPERIMENT_STATE_EXPLORE = 1;
const int PASS_THROUGH_EXPERIMENT_STATE_MEASURE_NEW_SCORE = 2;
/**
 * - a successful, safe improvement only needs to be:
 *   - no score impact passthrough (but potentially information)
 *   - successful branch
 */
const int PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT = 3;
const int PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST_EXISTING_SCORE = 4;
const int PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST_NEW_SCORE = 5;
const int PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND_EXISTING_SCORE = 6;
const int PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND_NEW_SCORE = 7;

const int PASS_THROUGH_EXPERIMENT_STATE_FAIL = 8;
const int PASS_THROUGH_EXPERIMENT_STATE_SUCCESS = 9;

class PassThroughExperimentOverallHistory;
class PassThroughExperiment : public AbstractExperiment {
public:
	std::vector<int> scope_context;
	std::vector<int> node_context;

	int state;
	int state_iter;
	int sub_state_iter;

	double existing_average_score;
	double existing_score_variance;

	double curr_score;
	std::vector<int> curr_step_types;
	std::vector<ActionNode*> curr_actions;
	std::vector<ScopeNode*> curr_existing_scopes;
	std::vector<ScopeNode*> curr_potential_scopes;
	int curr_exit_depth;
	AbstractNode* curr_exit_node;

	double best_score;
	std::vector<int> best_step_types;
	std::vector<ActionNode*> best_actions;
	std::vector<ScopeNode*> best_existing_scopes;
	std::vector<ScopeNode*> best_potential_scopes;
	int best_exit_depth;
	AbstractNode* best_exit_node;

	bool new_is_better;

	std::vector<double> o_target_val_histories;

	int branch_experiment_step_index;
	BranchExperiment* branch_experiment;




};

#endif /* PASS_THROUGH_EXPERIMENT_H */