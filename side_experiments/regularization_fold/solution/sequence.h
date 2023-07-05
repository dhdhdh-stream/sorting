#ifndef SEQUENCE_H
#define SEQUENCE_H

const int SEQUENCE_INPUT_INIT_NONE = 0;
const int SEQUENCE_INPUT_INIT_LOCAL = 1;
const int SEQUENCE_INPUT_INIT_PREVIOUS = 2;
const int SEQUENCE_INPUT_INIT_LAST_SEEN = 3;
// if empty (i.e., not initialize), don't include in input_init_types, etc.

class Sequence {
public:
	BranchExperiment* experiment;
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

	std::vector<int> input_init_types;
	std::vector<int> input_init_target_layers;
	std::vector<int> input_init_target_indexes;
	/**
	 * - needs to be state that isn't passed down further
	 * - don't reuse, even between different sequences
	 *   - instead, use previous
	 * - negative indexing from end
	 */
	std::vector<int> input_init_local_scope_depths;
	std::vector<int> input_init_local_input_indexes;
	/**
	 * - if use previous, then take responsibility for setting value back
	 * 
	 * - if previous was local, then use original local class
	 *   - so transformation goes from original local to inner_input
	 *     - (so skipping previous input_inner_classes)
	 */
	std::vector<int> input_init_previous_step_indexes;
	std::vector<int> input_init_previous_input_indexes;
	std::vector<ClassDefinition*> input_init_last_seen_classes;
	std::vector<Transformation*> input_transformations;

	// TODO: will need to add new state

	std::vector<std::vector<int>> node_ids;
	/**
	 * - scope nodes always given empty context
	 *   - so will always result in reasonable sequence
	 *     - (though may not be optimal due to original scope's later explored early exits)
	 *   - even if from halfway activate
	 * 
	 * - don't include branch nodes (and won't have exit nodes)
	 *   - also on success, create new scopes rather than reuse original scopes
	 *     - but inner scopes will be reused and generalized
	 */

	std::map<int, std::vector<std::vector<StateNetwork*>>> state_networks;
	std::vector<std::vector<StateNetwork*>> step_state_networks;
	std::vector<std::vector<std::vector<std::vector<StateNetwork*>>>> sequence_state_networks;

	std::vector<int> input_furthest_layer_seen_in;
	std::vector<std::vector<bool>> input_steps_seen_in;

	std::vector<bool> input_is_new_class;

	std::vector<std::vector<int>> scope_additions_needed;
	std::vector<std::vector<std::pair<int, int>>> scope_node_additions_needed;

	void activate(std::vector<double>& flat_vals,
				  std::vector<ForwardContextLayer>& context,
				  BranchExperimentHistory* branch_experiment_history,
				  RunHelper& run_helper,
				  SequenceHistory* history);
};

class SequenceHistory {
public:
	Sequence* sequence;

	// just tracking here rather than in BranchExperimentHistory
	std::vector<std::vector<double>> step_input_vals_snapshots;
	std::vector<std::vector<StateNetworkHistory*>> step_state_network_histories;
	std::vector<std::vector<std::vector<std::vector<double>>>> sequence_input_vals_snapshots;
	std::vector<std::vector<std::vector<std::vector<StateNetworkHistory*>>>> sequence_state_network_histories;

	std::vector<std::vector<AbstractNodeHistory*>> node_histories;

};

#endif /* SEQUENCE_H */