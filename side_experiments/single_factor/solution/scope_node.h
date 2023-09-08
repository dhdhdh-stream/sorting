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
	 *     - so value is continued, but will have separate weights
	 */
	std::vector<int> input_target_layers;
	std::vector<int> input_target_indexes;
	// std::vector<int> input_is_negated;
	// TODO: add
	// TODO: get rid of checking to see if network should not be run
	// - not much gain, at the cost of potential a lot of accuracy
	//   - at most 10 networks, each with 30 operations
	std::vector<double> input_weight_mods;
	// TODO: because no longer updating state, don't need state weights and even ending scale after
	// - yeah, doesn't make sense when every branch breaks things
	// - instead, retrain new score network?
	//   - yeah, train a giant flat network over everything
	//     - or, train multiple train multiple flat networks for different scopes?
	//   - maybe train a giant flat network for initial score and after score too
	//     - check what the differences are
	//       - those differences are then what needs to be made available at branch
	//         - so then what's a scope?
	//           - can still have every new path be a scope
	//             - it's not that critical the exact split, just need some abstraction
	// - or go back to updating states?
	//   - but with normalization, cannot zero, so cannot prevent states from screwing up

	// - issue is that branch may invalidate all states after, not just states on path

	// - maybe pass branch state backwards and see when it stops becoming relevant?
	//   - then that's when can actually merge?
	//     - this is for off path scopes
	//       - on path can just modify/clear state

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

	Explore* explore;

	std::vector<int> experiment_hook_state_indexes;
	std::vector<int> experiment_hook_network_indexes;
	std::vector<std::vector<int>> experiment_hook_scope_contexts;
	std::vector<std::vector<int>> experiment_hook_node_contexts;
	std::vector<bool> experiment_hook_obs_is_pre;
	std::vector<int> experiment_hook_obs_indexes;

	std::vector<int> experiment_hook_test_indexes;
	std::vector<std::vector<int>> experiment_hook_test_scope_contexts;
	std::vector<std::vector<int>> experiment_hook_test_node_contexts;
	std::vector<bool> experiment_hook_test_obs_is_pre;
	std::vector<int> experiment_hook_test_obs_indexes;



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

	ScopeNodeHistory(ScopeNode* node);
	ScopeNodeHistory(ScopeNodeHistory* original);	// deep copy for seed
	~ScopeNodeHistory();
};

#endif /* SCOPE_NODE_H */