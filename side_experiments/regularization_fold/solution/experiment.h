#ifndef EXPERIMENT_H
#define EXPERIMENT_H

class Experiment {
public:
	std::vector<int> scope_context;
	std::vector<int> node_context;	// store explore node index in node_context[0]

	int num_inner_inputs;
	std::vector<double> init_inner_input_scope_depths;	// -1 if no init
	std::vector<double> init_inner_input_input_indexes;
	// add to and set at corresponding scope node
	// - after setting back, splits back into 2 separate variables
	//   - do not set back to 0.0, so units remain the same

	// TODO: think about rewinding memory
	// - current main issue is how to rewind to arrive at state different than what had been calculated?
	//   - otherwise, not much more to gain than local state
	// (rewinding might also be not calculating certain state until needed, but will also not worry about that, and calculate fully for now)
	// TODO: maybe add last seen instead?

	int sequence_length;
	std::vector<int> step_types;
	std::vector<Action> actions;
	std::vector<int> existing_scope_ids;
	std::vector<Fetch*> fetches;	// squash into new scope if experiment successful
	/**
	 * - -1 if nothing passed
	 * - shared for existing scopes and fetches
	 * - tracks scope sizes (if scopes later update)
	 */
	std::vector<std::vector<int>> inner_input_indexes;
	/**
	 * - what networks to use in inner
	 *   - may not match outer original type
	 * - NULL if use outer original type
	 */
	std::vector<std::vector<TypeDefinition*>> inner_input_types;
	std::vector<std::vector<Transformation*>> inner_input_transformations;
	std::vector<std::vector<int>> fetch_output_inner_input_indexes;

	Network* starting_score_network;

	/**
	 * - num_new_states == num_inner_inputs + 5
	 * - use inner inputs as well as their existing values
	 *   - don't pay attention to existing networks
	 *     - if needs to be new, fully redo
	 * 
	 * - index 0 is global
	 * - contexts in the middle
	 * - last index is inner
	 */
	std::vector<std::map<int, std::vector<std::vector<Network*>>>> action_node_state_networks;
	// temporary to determine state needed
	std::vector<std::map<int, std::vector<Network*>>> action_node_score_networks;

	std::vector<std::vector<Network*>> sequence_state_networks;
	std::vector<Network*> sequence_score_networks;
	std::vector<std::vector<std::vector<std::vector<Network*>>>> fetch_state_networks;
	std::vector<std::vector<std::vector<Network*>>> fetch_score_networks;

	std::map<ScopeNode*, std::vector<Network*>> scope_node_state_networks;
	// TODO: train against seed evaluating to 0.0
	// - or don't seed, but simply train against existing (perhaps before applying changes from inner)
	// TODO: calculate transformation between outer state and new

	double* existing_average_score;
	double* existing_score_variance;
	double* existing_average_misguess;
	double* existing_misguess_variance;

	double seed_start_predicted_score;
	double seed_start_scale_factor;
	std::vector<double> seed_state_vals_snapshot;
	ScopeHistory* seed_outer_context_history;
	double seed_target_val;
	// don't worry about sequence for seed, as due to updates for inner scope, seed may quickly become irrelevant

	double curr_branch_average_score;
	double curr_branch_existing_average_score;
	double curr_replace_average_score;
	double curr_replace_average_misguess;
	double curr_replace_misguess_variance;


};

class ExperimentHistory {
public:
	Experiment* experiment;

	bool can_zero;

	std::vector<double> starting_state_vals_snapshot;
	std::vector<double> starting_new_state_vals_snapshot;
	double existing_predicted_score;

	std::vector<double> sequence_obs_snapshots;
	std::vector<std::vector<double>> sequence_starting_new_state_vals_snapshots;
	std::vector<std::vector<bool>> sequence_network_zeroed;
	std::vector<std::vector<double>> sequence_ending_new_state_vals_snapshots;


};

#endif /* EXPERIMENT_H */