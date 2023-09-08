/**
 * - simply treat inputs as part of the sequence
 *   - i.e., don't try to improve upon after
 *     - may completely change behavior
 *       - making learning/measuring unstable
 *   - but combination did well at least once during explore
 *     - so even if not perfect, still good chance to be an improvement
 * 
 * - don't include pre_state_networks/post_state_networks
 */

#ifndef SEQUENCE_H
#define SEQUENCE_H

const int SEQUENCE_INPUT_TYPE_NONE = 0;
/**
 * - set back after so later sequences can simply continue using
 */
const int SEQUENCE_INPUT_TYPE_LOCAL = 1;
// TODO: add type previous, but it pulls from previous' inner scope

class SequenceHistory;
class Sequence {
public:
	/**
	 * - non-loop
	 */
	std::vector<Scope*> scopes;

	/**
	 * - starting_node_ids[0] == node_ids[0][0]
	 * 
	 * - last node id can be any type
	 *   - others all have to be non-loop scope nodes
	 */
	std::vector<int> starting_node_ids;

	std::vector<int> input_types;
	std::vector<int> input_target_layers;
	std::vector<int> input_target_indexes;
	/**
	 * - negative indexing from end
	 *   - original scope is 0
	 */
	std::vector<int> input_local_scope_depths;
	std::vector<int> input_local_input_indexes;
	// std::vector<bool> input_local_is_negated;
	// TODO: add
	std::vector<double> input_weight_mods;

	std::vector<std::vector<int>> node_ids;
	/**
	 * - scope nodes always given empty context
	 *   - so will always result in reasonable sequence
	 *     - (though may not be optimal due to original scope's later explored early exits)
	 */

	AbstractExperiment* experiment;
	int step_index;		// 0 if loop

	std::vector<std::vector<double>> ending_weight_mods;


};

class SequenceHistory {
public:
	Sequence* sequence;

	std::vector<std::vector<AbstractNodeHistory*>> node_histories;

	/**
	 * - i.e., post_obs_snapshot
	 *   - only need post_obs_snapshot as after new branch
	 */
	std::vector<std::vector<double>> initialized_locally_val_snapshots;
	std::vector<std::vector<double>> initialized_locally_weight_snapshots;

	SequenceHistory(Sequence* sequence);
	~SequenceHistory();
};

#endif /* SEQUENCE_H */