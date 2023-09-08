#ifndef ACTION_NODE_H
#define ACTION_NODE_H

class ActionNode : public AbstractNode {
public:
	// Action action;

	std::vector<int> state_network_indexes;
	std::vector<StateNetwork*> state_networks;

	int next_node_id;

	Explore* explore;

	std::vector<int> experiment_hook_state_indexes;
	std::vector<int> experiment_hook_network_indexes;
	std::vector<std::vector<int>> experiment_hook_scope_contexts;
	std::vector<std::vector<int>> experiment_hook_node_contexts;

	/**
	 * - -1 if unused
	 */
	int experiment_hook_test_index;
	std::vector<int> experiment_hook_test_scope_context;
	std::vector<int> experiment_hook_test_node_context;



};

class ActionNodeHistory : public AbstractNodeHistory {
public:
	double obs_snapshot;

	ActionNodeHistory(ActionNode* node);
	ActionNodeHistory(ActionNodeHistory* original);	// deep copy for seed
	~ActionNodeHistory();
};

#endif /* ACTION_NODE_H */