#ifndef SCOPE_NODE_H
#define SCOPE_NODE_H

class ScopeNode : public AbstractNode {
public:
	int inner_scope_id;

	std::vector<int> starting_node_ids;

	/**
	 * - -1 if not passed in
	 * 
	 * - if target_layer > 0, then still continue back out and resolve twice
	 *   - once in inner, and once on continuation
	 */
	std::vector<int> input_target_layers;
	std::vector<int> input_target_indexes;

	/**
	 * - state that isn't passed in to top layer
	 *   - so can include state that is skip passed
	 *     - may result in there being duplicate information
	 *       - but also makes sure information is available past skip
	 *       - same applies for state_initialized_locally/post_obs_snapshot
	 */
	std::vector<int> pre_state_network_indexes;
	/**
	 * - inner scope state index
	 *   - so target_layer == 0
	 */
	std::vector<int> pre_state_network_target_indexes;
	std::vector<StateNetwork*> pre_state_networks;

	Scale* scope_scale_mod;

	/**
	 * - don't activate if early exit
	 * 
	 * - post_obs_snapshot index
	 *   - i.e., not inner scope state index
	 */
	std::vector<int> post_state_network_indexes;
	std::vector<int> post_state_network_target_indexes;
	std::vector<StateNetwork*> post_state_networks;

	int next_node_id;



};

class ScopeNodeHistory : public AbstractNodeHistory {
public:
	/**
	 * - state that isn't passed in
	 */
	std::vector<double> pre_obs_snapshot;

	ScopeHistory* inner_scope_history;

	/**
	 * - state initialized in inner
	 */
	std::vector<double> post_obs_snapshot;

	AbstractExperimentHistory* experiment_history;

	ScopeNodeHistory(ScopeNode* node);
	ScopeNodeHistory(ScopeNodeHistory* original);	// deep copy for seed
	~ScopeNodeHistory();
};

#endif /* SCOPE_NODE_H */