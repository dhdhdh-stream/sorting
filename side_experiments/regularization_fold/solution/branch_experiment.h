#ifndef BRANCH_EXPERIMENT_H
#define BRANCH_EXPERIMENT_H

const int BRANCH_EXPERIMENT_STATE_EXPERIMENT = 0;
const int BRANCH_EXPERIMENT_STATE_NEW_TYPES = 1;
const int BRANCH_EXPERIMENT_STATE_CLEANUP = 2;

const int BRANCH_EXPERIMENT_RESULT_FAIL = 0;
const int BRANCH_EXPERIMENT_RESULT_REPLACE = 1;
const int BRANCH_EXPERIMENT_RESULT_BRANCH = 2;

class BranchExperiment {
public:
	int num_steps;
	std::vector<int> step_types;
	std::vector<Action> actions;
	std::vector<Sequence*> sequences;

	ScoreNetwork* starting_score_network;

	std::vector<std::vector<StateNetwork*>> step_state_networks;
	std::vector<ScoreNetwork*> step_score_networks;

	int exit_depth;
	std::vector<std::map<StateDefinition*, ExitNetwork*>> exit_networks;

	double* existing_average_score;
	double* existing_score_variance;
	double* existing_average_misguess;
	double* existing_misguess_variance;

	double seed_start_predicted_score;
	double seed_start_scale_factor;
	std::vector<double> seed_state_vals_snapshot;
	ScopeHistory* seed_outer_context_history;
	double seed_target_val;
	// only worry about starting score network for seed, as due to updates for inner scopes, seed may quickly become irrelevant

	double curr_branch_average_score;
	double curr_branch_existing_average_score;
	double curr_replace_average_score;
	double curr_replace_average_misguess;
	double curr_replace_misguess_variance;

	int state;
	int state_iter;
	double sum_error;

	std::vector<bool> input_is_new_type;

	std::vector<int> input_additions_needed;
	std::vector<std::pair<int, int>> scope_node_additions_needed;

};

class BranchExperimentHistory {
public:
	BranchExperiment* branch_experiment;

	std::vector<double> starting_state_vals_snapshot;
	std::vector<double> starting_new_state_vals_snapshot;
	double existing_predicted_score;

	std::vector<double> step_obs_snapshots;
	std::vector<std::vector<double>> step_starting_new_state_vals_snapshots;
	std::vector<std::vector<StateNetworkHistory*>> step_state_network_histories;
	std::vector<std::vector<double>> step_ending_new_state_vals_snapshots;
	std::vector<ScoreNetworkHistory*> step_score_network_histories;
	std::vector<double> step_score_network_outputs;

	std::vector<SequenceHistory*> sequence_histories;

	std::vector<std::vector<double>> exit_state_vals_snapshot;
	std::vector<ExitNetworkHistory*> exit_network_histories;
};

#endif /* BRANCH_EXPERIMENT_H */