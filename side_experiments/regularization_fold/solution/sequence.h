#ifndef SEQUENCE_H
#define SEQUENCE_H

class Sequence {
public:
	std::vector<Scope*> scopes;

	/**
	 * - for node_ids[0][0]
	 *   - if not empty, nodes_ids[0][0] has to be scope node
	 * - last node id can be any type
	 *   - others all have to be scope nodes
	 */
	std::vector<int> starting_node_ids;

	std::vector<int> last_seen_init_new_state_index;
	std::vector<StateDefinition*> last_seen_init_types;
	/**
	 * - for last seen, use fully initially, but gradually scale down impact
	 *   - so new state can grow into replacing use of last seen
	 */
	std::vector<Transformation*> last_seen_init_transformations;

	std::vector<std::vector<int>> input_indexes;
	std::vector<std::vector<int>> input_target_indexes;
	/**
	 * - what networks to use in inner
	 *   - may not match outer original type
	 * - NULL if use outer original type
	 */
	std::vector<std::vector<StateDefinition*>> input_types;
	std::vector<std::vector<Transformation*>> input_transformations;

	std::vector<std::vector<int>> node_ids;
	/**
	 * - scope nodes always given empty context
	 *   - so will always result in reasonable sequence
	 *     - (though may not be optimal due to original scope's later explored early exits)
	 * 
	 * - don't include branch nodes
	 *   - also on success, create new scopes rather than reuse original scopes
	 *     - but inner scopes will be reused and generalized
	 */

	// rely on last seen to make states accessible outside

	// initialize on Experiment
	std::vector<std::vector<std::vector<Network*>>> rising_state_networks;
	std::vector<std::vector<Network*>> rising_score_networks;
	std::vector<std::vector<std::vector<Network*>>> falling_state_networks;
	std::vector<std::vector<Network*>> falling_score_networks;

	void explore_activate(std::vector<double>& flat_vals,
						  std::vector<double>& new_state_vals,
						  RunHelper& run_helper);

	void experiment_activate(int experiment_step_index,
							 std::vector<double>& flat_vals,
							 std::vector<double>& input_vals,
							 std::vector<StateDefinition*>& input_types,
							 RunHelper& run_helper,
							 SequenceHistory* history);
};

class SequenceHistory {
public:
	std::vector<std::vector<AbstractNodeHistory*>> rising_node_histories;
	std::vector<std::vector<AbstractNodeHistory*>> falling_node_histories;


};

#endif /* SEQUENCE_H */