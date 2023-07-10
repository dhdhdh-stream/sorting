#ifndef BRANCH_EXPERIMENT_H
#define BRANCH_EXPERIMENT_H

const int BRANCH_EXPERIMENT_STEP_TYPE_ACTION = 0;
const int BRANCH_EXPERIMENT_STEP_TYPE_SEQUENCE = 0;

const int BRANCH_EXPERIMENT_RESULT_FAIL = 0;
const int BRANCH_EXPERIMENT_RESULT_REPLACE = 1;
const int BRANCH_EXPERIMENT_RESULT_BRANCH = 2;

class BranchExperiment : public Experiment {
public:
	int num_steps;
	std::vector<int> step_types;
	std::vector<Action> actions;
	std::vector<Sequence*> sequences;

	int exit_depth;
	int exit_node_id;

	ScoreNetwork* starting_score_network;
	ScoreNetwork* starting_original_score_network;

	std::vector<std::vector<StateNetwork*>> step_state_networks;
	std::vector<ScoreNetwork*> step_score_networks;
	/**
	 * - share networks instead of having separate networks for sequence
	 *   - much cleaner when merging new state
	 *     - can be seen as generalization anyways
	 */

	Scale* sequence_scale_factors;

	/**
	 * - exit node takes place after new experiment scope
	 *   - so new experiment scope has a default ending
	 *   - so doesn't include experiment context
	 *     - so layer is actually this->scopes.size()
	 */
	std::vector<ExitNetwork*> exit_networks;
	std::vector<double> exit_network_impacts;

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

	std::map<int, vector<bool>> scope_steps_seen_in;

	std::vector<std::vector<int>> scope_additions_needed;
	std::vector<std::vector<std::pair<int, int>>> scope_node_additions_needed;



};

class BranchExperimentHistory {
public:
	BranchExperiment* experiment;

	std::vector<double> starting_state_vals_snapshot;
	std::vector<double> starting_new_state_vals_snapshot;
	double existing_predicted_score;

	bool is_original;
	ScoreNetworkHistory* score_network_history;
	double score_network_update;

	std::vector<double> step_obs_snapshots;
	std::vector<std::vector<double>> step_starting_new_state_vals_snapshots;
	std::vector<std::vector<StateNetworkHistory*>> step_state_network_histories;
	std::vector<std::vector<double>> step_ending_new_state_vals_snapshots;
	std::vector<ScoreNetworkHistory*> step_score_network_histories;
	std::vector<double> step_score_network_outputs;

	std::vector<SequenceHistory*> sequence_histories;
	std::vector<std::vector<double>> sequence_ending_input_vals_snapshots;

	std::vector<std::vector<double>> exit_state_vals_snapshot;
	std::vector<double> exit_new_state_vals_snapshot;
	std::vector<ExitNetworkHistory*> exit_network_histories;
};

#endif /* BRANCH_EXPERIMENT_H */