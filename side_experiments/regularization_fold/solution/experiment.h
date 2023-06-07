#ifndef EXPERIMENT_H
#define EXPERIMENT_H

class Experiment {
public:
	std::vector<int> scope_context;
	std::vector<int> node_context;	// store explore node index in node_context[0]

	int num_inner_inputs;
	std::vector<double> init_inner_input_scope_depths;	// -1 if no init
	std::vector<double> init_inner_input_input_indexes;
	std::vector<Transformation*> init_inner_input_transformations;

	// TODO: think about rewinding memory
	// - current main issue is how to rewind to arrive at state different than what had been calculated?
	//   - otherwise, not much more to gain than local state
	// (rewinding might also be not calculating certain state until needed, but will also not worry about that, and calculate fully for now)
	// TODO: maybe add last seen for now

	int sequence_length;
	std::vector<int> step_type;
	std::vector<Action> actions;
	std::vector<int> existing_scope_ids;
	std::vector<Fetch*> fetches;	// squash into new scope if experiment successful
	std::vector<std::vector<int>> inner_input_indexes;	// shared for existing scopes and fetches
	std::vector<std::vector<int>> fetch_output_inner_input_indexes;

	Network* starting_score_network;

	// num_new_states == num_inner_inputs + 5
	/**
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


};

#endif /* EXPERIMENT_H */