#ifndef ACTION_NODE_H
#define ACTION_NODE_H

class ActionNode : public AbstractNode {
public:
	bool is_off;

	Action action;

	std::vector<int> state_scope_contexts;
	std::vector<int> state_node_contexts;
	std::vector<int> state_indexes;
	std::vector<int> state_obs_indexes;
	std::vector<State*> state_defs;
	std::vector<int> state_network_indexes;


	bool experiment_is_off;


};

#endif /* ACTION_NODE_H */