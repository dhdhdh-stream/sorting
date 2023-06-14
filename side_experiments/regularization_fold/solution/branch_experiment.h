#ifndef BRANCH_EXPERIMENT_H
#define BRANCH_EXPERIMENT_H

class BranchExperiment {
public:
	int num_steps;
	std::vector<int> step_types;
	std::vector<Action> actions;
	std::vector<Sequence*> sequences;

	/**
	 * - num_new_states tracked in AbstractExperiment
	 *   - equals needed inner input + 5
	 */
	std::vector<double> local_init_scope_depths;	// -1 if no init
	std::vector<double> local_init_input_indexes;
	/**
	 * - add to and set at corresponding scope node
	 *   - after setting back, splits back into 2 separate variables
	 *     - do not set back to 0.0, so units remain the same
	 *   - if not initialized, just leave at 0.0
	 */

	// TODO: think about rewinding memory
	// - current main issue is how to rewind to arrive at state different than what had been calculated?
	//   - otherwise, not much more to gain than local state
	// ("rewinding" could instead be not calculating certain state until needed, but will also not worry about that, and calculate fully for now)

	Network* starting_score_network;

	std::vector<std::vector<Network*>> step_state_networks;
	std::vector<Network*> step_score_networks;

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


};

class BranchExperimentHistory {
public:
	BranchExperiment* branch_experiment;

	std::vector<double> starting_state_vals_snapshot;
	std::vector<double> starting_new_state_vals_snapshot;
	double existing_predicted_score;

	std::vector<double> sequence_obs_snapshots;
	std::vector<std::vector<double>> sequence_starting_new_state_vals_snapshots;
	std::vector<std::vector<bool>> sequence_network_zeroed;
	std::vector<std::vector<double>> sequence_ending_new_state_vals_snapshots;


};

#endif /* BRANCH_EXPERIMENT_H */