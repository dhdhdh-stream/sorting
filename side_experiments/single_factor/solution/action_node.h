#ifndef ACTION_NODE_H
#define ACTION_NODE_H

class ActionNode : public AbstractNode {
public:
	// Action action;

	std::vector<int> target_layer;
	std::vector<int> target_indexes;
	std::vector<StateNetwork*> state_networks;
};

#endif /* ACTION_NODE_H */