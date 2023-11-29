#ifndef LOOP_EXPERIMENT_H
#define LOOP_EXPERIMENT_H

const int LOOP_EXPERIMENT_STATE_TRAIN_EXISTING = 0;
const int LOOP_EXPERIMENT_STATE_EXPLORE = 1;

const int LOOP_EXPERIMENT_STATE_TRAIN_PRE = 2;
const int LOOP_EXPERIMENT_STATE_TRAIN = 3;
const int LOOP_EXPERIMENT_SUB_STATE_TRAIN_HALT = 0;
const int LOOP_EXPERIMENT_SUB_STATE_TRAIN_CONTINUE = 1;

const int LOOP_EXPERIMENT_STATE_MEASURE = 4;
const int LOOP_EXPERIMENT_STATE_VERIFY_EXISTING = 5;
const int LOOP_EXPERIMENT_STATE_VERIFY = 6;
const int LOOP_EXPERIMENT_STATE_CAPTURE_VERIFY = 7;

const int LOOP_EXPERIMENT_STATE_FAIL = 8;
const int LOOP_EXPERIMENT_STATE_SUCCESS = 9;

/**
 * - don't train/evaluate past limit to see if has better score
 *   - so won't train to go past limit (to receive -1.0 score)
 *     - but hopefully, after finalized, can generalize to go past limit when correct
 */
const int TRAIN_ITER_LIMIT = 8;
const int NUM_SAMPLES_MULTIPLIER = 2;

class LoopExperimentOverallHistory;
class LoopExperiment : public AbstractExperiment {
public:
	double average_instances_per_run;

	int state;
	int sub_state;
	int state_iter;
	int sub_state_iter;

	PotentialScopeNode* potential_loop;

	double existing_average_score;
	double existing_score_variance;

	std::vector<std::map<int, double>> start_input_state_weights;
	std::vector<std::map<int, double>> start_local_state_weights;
	std::vector<std::map<State*, double>> start_temp_state_weights;

	std::vector<std::map<int, double>> halt_input_state_weights;
	std::vector<std::map<int, double>> halt_local_state_weights;
	std::vector<std::map<State*, double>> halt_temp_state_weights;
	int halt_constant;

	std::vector<std::map<int, double>> continue_input_state_weights;
	std::vector<std::map<int, double>> continue_local_state_weights;
	std::vector<std::map<State*, double>> continue_temp_state_weights;
	int continue_constant;

	std::vector<State*> new_states;
	std::vector<std::vector<ActionNode*>> new_state_nodes;
	std::vector<std::vector<std::vector<int>>> new_state_scope_contexts;
	std::vector<std::vector<std::vector<int>>> new_state_node_contexts;
	std::vector<std::vector<int>> new_state_obs_indexes;

	double measure_score;
	int measure_num_instances;
	int measure_sum_iters;

	std::vector<double> o_target_val_histories;
	std::vector<ScopeHistory*> i_scope_histories;
	std::vector<std::vector<std::map<int, StateStatus>>> i_input_state_vals_histories;
	std::vector<std::vector<std::map<int, StateStatus>>> i_local_state_vals_histories;
	std::vector<std::vector<std::map<State*, StateStatus>>> i_temp_state_vals_histories;
	std::vector<double> i_target_val_histories;
	std::vector<double> i_start_predicted_score_histories;

	std::vector<Problem> verify_problems;
	std::vector<double> verify_continue_scores;
	std::vector<double> verify_halt_scores;
	std::vector<std::vector<double>> verify_factors;

};

class LoopExperimentInstanceHistory : public AbstractExperimentHistory {
public:
	std::vector<PotentialScopeNodeHistory*> iter_histories;

	LoopExperimentInstanceHistory(LoopExperiment* experiment);
	LoopExperimentInstanceHistory(LoopExperimentInstanceHistory* original);
	~LoopExperimentInstanceHistory();
};

class LoopExperimentOverallHistory : public AbstractExperimentHistory {
public:
	int instance_count;

	bool has_target;
	double start_predicted_score;
	std::vector<double> halt_predicted_scores;

	LoopExperimentOverallHistory(LoopExperiment* experiment);
};

#endif /* LOOP_EXPERIMENT_H */