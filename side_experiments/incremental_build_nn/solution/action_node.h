#ifndef ACTION_NODE_H
#define ACTION_NODE_H

class ActionNodeHistory;
class ActionNode : public AbstractNode {
public:
	Action action;

	int next_node_id;
	AbstractNode* next_node;

	std::vector<int> hook_indexes;
	std::vector<std::vector<Scope*>> hook_scope_contexts;
	std::vector<std::vector<AbstractNode*>> hook_node_contexts;

	AbstractExperiment* experiment;


};

class ActionNodeHistory : public AbstractNodeHistory {
public:
	double obs_snapshot;

	ActionNodeHistory(ActionNode* node);
	ActionNodeHistory(ActionNodeHistory* original);
	~ActionNodeHistory();
};

#endif /* ACTION_NODE_H */