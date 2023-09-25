#ifndef ACTION_NODE_H
#define ACTION_NODE_H

class ActionNode : public AbstractNode {
public:
	// Action action;

	std::vector<bool> state_is_local;
	std::vector<int> state_ids;
	std::vector<State*> state_defs;
	/**
	 * - use id when saving/loading, but have direct reference for running
	 */
	std::vector<int> state_network_indexes;

	std::vector<std::vector<int>> score_state_scope_contexts;
	std::vector<std::vector<int>> score_state_node_contexts;
	/**
	 * - for top scope context
	 */
	std::vector<State*> score_state_defs;
	std::vector<int> score_state_network_indexes;

	std::vector<std::vector<int>> experiment_hook_score_state_scope_contexts;
	std::vector<std::vector<int>> experiment_hook_score_state_node_contexts;
	std::vector<State*> experiment_hook_score_state_defs;
	std::vector<int> experiment_hook_score_state_network_indexes;

	std::vector<int> test_hook_scope_contexts;
	std::vector<int> test_hook_node_contexts;
	int test_hook_index;

	int next_node_id;



};

class ActionNodeHistory : public AbstractNodeHistory {
public:
	double obs_snapshot;


};

#endif /* ACTION_NODE_H */