#ifndef ACTION_NODE_H
#define ACTION_NODE_H

class ActionNode : public AbstractNode {
public:
	// Action action;

	int next_node_id;

	std::vector<int> state_ids;
	std::vector<StateNetwork*> state_networks;

	std::vector<std::vector<int>> score_state_scope_contexts;
	std::vector<std::vector<int>> score_state_node_contexts;
	std::vector<int> score_state_ids;
	std::vector<StateNetwork*> score_state_networks;

	int experiment_hook_index;
	std::vector<int> experiment_hook_scope_contexts;
	std::vector<int> experiment_hook_node_contexts;

};

class ActionNodeHistory : public AbstractNodeHistory {
public:
	double obs_snapshot;

	
};

#endif /* ACTION_NODE_H */