#ifndef SEQUENCE_H
#define SEQUENCE_H

class Sequence {
public:
	/**
	 * - includes topmost layer
	 * - run from last to first
	 */
	std::vector<Scope*> rising_scopes;
	/**
	 * - may be empty
	 *   - i.e., doesn't include topmost layer
	 * - run from first to last
	 */
	std::vector<Scope*> falling_scopes;

	std::vector<std::vector<int>> rising_node_ids;
	std::vector<std::vector<int>> falling_node_ids;
	/**
	 * - only action nodes and scope nodes
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

	void activate(int experiment_step_index,
				  std::vector<double>& flat_vals,
				  std::vector<double>& state_vals,
				  std::vector<TypeDefinition*>& state_types,
				  RunHelper& run_helper,
				  SequenceHistory* history);
};

class SequenceHistory {
public:

	std::vector<std::vector<double>> obs_snapshots;
	
};

#endif /* SEQUENCE_H */