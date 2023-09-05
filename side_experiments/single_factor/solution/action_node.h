#ifndef ACTION_NODE_H
#define ACTION_NODE_H

class ActionNode : public AbstractNode {
public:
	// Action action;

	std::vector<int> state_network_indexes;
	std::vector<StateNetwork*> state_networks;

	int next_node_id;

};

class ActionNodeHistory : public AbstractNodeHistory {
public:
	double obs_snapshot;

	AbstractExperimentHistory* experiment_history;

	ActionNodeHistory(ActionNode* node);
	ActionNodeHistory(ActionNodeHistory* original);	// deep copy for seed
	~ActionNodeHistory();
};

#endif /* ACTION_NODE_H */