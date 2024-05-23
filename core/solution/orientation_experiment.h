/**
 * - branch experiment
 *   - no need to worry about passthroughs as would be more relevantly handled by eval sequence
 */

#ifndef ORIENTATION_EXPERIMENT_H
#define ORIENTATION_EXPERIMENT_H

const int ORIENTATION_EXPERIMENT_STATE_TRAIN_EXISTING = 0;
const int ORIENTATION_EXPERIMENT_STATE_EXPLORE_MISGUESS = 1;
/**
 * - also gathers samples to train new
 */
const int ORIENTATION_EXPERIMENT_STATE_EXPLORE_IMPACT = 2;
const int ORIENTATION_EXPERIMENT_STATE_MEASURE = 3;



#if defined(MDEBUG) && MDEBUG
const int ORIENTATION_EXPERIMENT_EXPLORE_ITERS = 2;
#else
const int ORIENTATION_EXPERIMENT_EXPLORE_ITERS = 100;
#endif /* MDEBUG */

class OrientationExperimentHistory;
class OrientationExperiment : public AbstractExperiment {
public:
	int state;
	int state_iter;
	int explore_iter;

	double existing_average_score;
	double existing_score_standard_deviation;

	std::vector<std::vector<Scope*>> input_scope_contexts;
	std::vector<std::vector<AbstractNode*>> input_node_contexts;
	std::vector<int> input_obs_indexes;

	std::vector<double> existing_linear_weights;
	std::vector<std::vector<int>> existing_network_input_indexes;
	Network* existing_network;
	double existing_average_misguess;
	double existing_misguess_standard_deviation;

	std::vector<int> step_types;
	std::vector<ActionNode*> curr_actions;
	std::vector<ScopeNode*> curr_scopes;
	AbstractNode* exit_next_node;

	ActionNode* ending_node;

	double new_average_score;

	std::vector<double> new_linear_weights;
	std::vector<std::vector<int>> new_network_input_indexes;
	Network* new_network;
	double new_average_misguess;
	double new_misguess_standard_deviation;



	std::vector<ScopeHistory*> scope_histories;

};

class OrientationExperimentHistory : public AbstractExperimentHistory {
public:
	EvalHistory* outer_eval_history;

	double existing_predicted_score;

};

#endif /* ORIENTATION_EXPERIMENT_H */