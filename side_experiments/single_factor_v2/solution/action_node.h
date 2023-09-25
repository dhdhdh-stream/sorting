#ifndef ACTION_NODE_H
#define ACTION_NODE_H

class ActionNode : public AbstractNode {
public:
	// Action action;

	int next_node_id;

	std::vector<int> local_state_ids;
	std::vector<int> obs_ids;
	/**
	 * - -1 if actual obs
	 *   - otherwise, has to be state that has just previously been updated
	 *     - so networks need to be activated in order
	 */
	std::vector<State*> states;
	/**
	 * - -1 if sum (i.e., within loop)
	 */
	std::vector<int> network_indexes;

	std::vector<int> local_loop_sum_id;
	std::vector<int> loop_sum_obs_ids;
	std::vector<int> loop_sum_iter_indexes;
	/**
	 * - loop scopes in charge of maintaining iter_index
	 */

	std::vector<std::vector<int>> score_state_scope_contexts;
	std::vector<std::vector<int>> score_state_node_contexts;
	std::vector<State*> score_states;
	/**
	 * - use id when saving/loading, but have direct reference when running
	 */
	std::vector<int> score_obs_ids;
	std::vector<int> score_network_indexes;
	std::vector<int> score_loop_iter;
	std::vector<int> score_context_index;

	std::vector<std::vector<int>> experiment_hook_score_state_scope_contexts;
	std::vector<std::vector<int>> experiment_hook_score_state_node_contexts;
	std::vector<State*> experiment_hook_score_states;
	std::vector<int> experiment_hook_score_obs_ids;
	std::vector<int> experiment_hook_score_network_indexes;

	std::vector<int> test_hook_scope_contexts;
	std::vector<int> test_hook_node_contexts;
	int test_hook_obs_id;
	int test_hook_index;

};

class ActionNodeHistory : public AbstractNodeHistory {
public:
	double obs_snapshot;
	std::vector<double> state_snapshots;
	
};

#endif /* ACTION_NODE_H */