#ifndef SEQUENCE_H
#define SEQUENCE_H

const int SEQUENCE_INPUT_INIT_NONE = 0;
const int SEQUENCE_INPUT_INIT_LOCAL = 1;
const int SEQUENCE_INPUT_INIT_PREVIOUS = 2;
const int SEQUENCE_INPUT_INIT_LAST_SEEN = 3;

class Sequence {
public:
	int step_index;

	/**
	 * - non-loop
	 */
	std::vector<Scope*> scopes;

	/**
	 * - for node_ids[0][0]
	 *   - if not empty, nodes_ids[0][0] has to be scope node
	 * - last node id can be any type
	 *   - others all have to be non-loop scope nodes
	 * - length is scopes-1
	 */
	std::vector<int> starting_node_ids;

	std::vector<std::vector<int>> input_init_types;
	// needs to be state that isn't passed down further
	std::vector<std::vector<int>> input_init_local_scope_depths;
	std::vector<std::vector<int>> input_init_local_input_indexes;
	/**
	 * - if use previous, then take responsibility for setting value back
	 *   - during experiment, gradually scale down impact to instead rely on exit networks
	 */
	std::vector<std::vector<int>> input_init_previous_step_index;
	std::vector<std::vector<int>> input_init_previous_scope_index;
	std::vector<std::vector<int>> input_init_previous_input_index;
	std::vector<std::vector<StateDefinition*>> input_init_last_seen_types;
	std::vector<std::vector<Transformation*>> input_init_transformations;
	/**
	 * - what networks to use in inner
	 *   - may not match outer original type
	 * - NULL if use outer original type
	 */
	std::vector<std::vector<StateDefinition*>> input_inner_types;

	std::vector<std::vector<int>> node_ids;
	/**
	 * - scope nodes always given empty context
	 *   - so will always result in reasonable sequence
	 *     - (though may not be optimal due to original scope's later explored early exits)
	 * 
	 * - don't include branch nodes (and won't have exit nodes)
	 *   - also on success, create new scopes rather than reuse original scopes
	 *     - but inner scopes will be reused and generalized
	 */

	std::map<int, std::vector<std::vector<std::vector<StateNetwork*>>>> state_networks;
	std::vector<std::vector<std::vector<StateNetwork*>>> step_state_networks;
	std::vector<std::vector<std::vector<std::vector<std::vector<StateNetwork*>>>>> sequence_state_networks;

	void explore_activate(std::vector<double>& flat_vals,
						  std::vector<double>& new_state_vals,
						  std::vector<StateDefinition>& new_state_types,
						  RunHelper& run_helper);

	void experiment_activate(std::vector<double>& flat_vals,
							 RunHelper& run_helper,
							 SequenceHistory* history);
};

class SequenceHistory {
public:
	Sequence* sequence;

	// just tracking here rather than in BranchExperimentHistory
	std::vector<std::vector<std::vector<double>>> step_input_vals_snapshots;
	std::vector<std::vector<std::vector<StateNetworkHistory*>>> step_state_network_histories;
	std::vector<std::vector<std::vector<std::vector<std::vector<double>>>>> sequence_input_vals_snapshots;
	std::vector<std::vector<std::vector<std::vector<std::vector<StateNetworkHistory*>>>>> sequence_state_network_histories;

	std::vector<std::vector<AbstractNodeHistory*>> node_histories;

};

#endif /* SEQUENCE_H */