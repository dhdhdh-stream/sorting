#ifndef EVAL_EXPERIMENT_H
#define EVAL_EXPERIMENT_H

const int EVAL_EXPERIMENT_STATE_CAPTURE_EXISTING = 0;
const int EVAL_EXPERIMENT_STATE_EXPLORE = 1;
const int EVAL_EXPERIMENT_STATE_CAPTURE_NEW = 2;
const int EVAL_EXPERIMENT_STATE_MEASURE = 3;
const int EVAL_EXPERIMENT_STATE_VERIFY_1ST_EXISTING = 4;
const int EVAL_EXPERIMENT_STATE_VERIFY_1ST = 5;
const int EVAL_EXPERIMENT_STATE_VERIFY_2ND_EXISTING = 6;
const int EVAL_EXPERIMENT_STATE_VERIFY_2ND = 7;

class EvalExperimentHistory;
class EvalExperiment : public AbstractExperiment {
public:
	int state;
	int state_iter;
	int sub_state_iter;

	double original_average_score;
	double original_score_standard_deviation;
	double original_average_misguess;
	double original_misguess_standard_deviation;

	std::vector<ScopeHistory*> existing_decision_scope_histories;
	std::vector<ScopeHistory*> existing_final_scope_histories;
	std::vector<double> existing_target_val_histories;

	double new_score;
	std::vector<ActionNode*> actions;
	AbstractNode* exit_next_node;

	std::vector<ScopeHistory*> new_decision_scope_histories;
	std::vector<ScopeHistory*> new_final_scope_histories;
	std::vector<double> new_target_val_histories;

	/**
	 * - use original_average_score as new eval average_score
	 */

	std::vector<std::vector<AbstractNode*>> eval_input_node_contexts;
	std::vector<int> eval_input_obs_indexes;

	std::vector<double> eval_linear_weights;
	std::vector<std::vector<int>> eval_network_input_indexes;
	Network* eval_network;
	double eval_average_misguess;
	double eval_misguess_standard_deviation;

	std::vector<std::vector<AbstractNode*>> decision_input_node_contexts;
	std::vector<int> decision_input_obs_indexes;

	double existing_average_score;
	double existing_score_standard_deviation;

	std::vector<double> existing_linear_weights;
	std::vector<std::vector<int>> existing_network_input_indexes;
	Network* existing_network;
	double existing_network_average_misguess;
	double existing_network_misguess_standard_deviation;

	double new_average_score;

	std::vector<double> new_linear_weights;
	std::vector<std::vector<int>> new_network_input_indexes;
	Network* new_network;
	double new_network_average_misguess;
	double new_network_misguess_standard_deviation;

	double combined_misguess;
	int original_count;
	int branch_count;

	bool is_pass_through;

};

class EvalExperimentHistory {
public:
	std::vector<double> predicted_scores;

	EvalExperimentHistory(EvalExperiment* experiment);
};

#endif /* EVAL_EXPERIMENT_H */