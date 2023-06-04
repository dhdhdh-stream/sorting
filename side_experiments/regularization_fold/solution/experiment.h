#ifndef EXPERIMENT_H
#define EXPERIMENT_H

class Experiment {
public:
	std::vector<int> scope_context;
	std::vector<int> node_context;	// store explore node index in node_context[0]

	int sequence_length;
	std::vector<bool> is_inner_scope;
	std::vector<int> existing_scope_ids;
	int num_inner_inputs;
	std::vector<double> inner_input_scope_depths;	// -1 for new
	std::vector<double> inner_input_input_indexes;	// new_state_index for new
	std::vector<std::vector<int>> existing_scope_inputs;
	std::vector<Action> actions;

	int num_new_states;	// new inner inputs + 5
	/**
	 * - index 0 is global
	 * - contexts in the middle
	 * - last index is inner
	 */
	std::vector<std::map<int, std::vector<std::vector<Network*>>>> action_node_state_networks;
	// temporary to determine state needed
	std::vector<std::map<int, std::vector<Network*>>> action_node_score_networks;
	std::vector<std::vector<Network*>> scope_node_state_networks;

	Network* starting_score_network;

	std::vector<int> local_inner_input_mapping;
	std::vector<int> reverse_local_inner_input_mapping;

	// TODO: add map to update all inputs?
	// TODO: can also make all score networks temporary to measure misguess
	// - then separately, update types?
	// TODO: can also not backprop errors on most score networks, and only backprop errors on "key" networks?

	// or, can make the assumption that if is not passed into inner, then local state does not matter
	// and later on, sequence doesn't matter relative to inner
	// - so can ignore outer state?

	// TODO: practice extending state

	/**
	 * - new_states
	 * - inner input local
	 */
	std::vector<std::vector<Network*>> sequence_state_networks;
	std::vector<Network*> sequence_score_networks;


};

class ExperimentHistory {
public:
	Experiment* Experiment;

	bool can_zero;
	// TODO: 10% zero
	// - should be enough to converge because a step cannot depend on any particular future step

	std::vector<double> starting_state_vals_snapshot;
	std::vector<double> starting_new_state_vals_snapshot;
	double existing_predicted_score;
};

#endif /* EXPERIMENT_H */