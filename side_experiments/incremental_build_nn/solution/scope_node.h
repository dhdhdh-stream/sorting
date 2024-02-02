#ifndef SCOPE_NODE_H
#define SCOPE_NODE_H

class ScopeNodeHistory;
class ScopeNode : public AbstractNode {
public:
	Scope* scope;

	int next_node_id;
	AbstractNode* next_node;

	AbstractExperiment* experiment;

};

class ScopeNodeHistory : public AbstractNodeHistory {
public:
	ScopeHistory* scope_history;

	AbstractExperimentHistory* experiment_history;

	ScopeNodeHistory(ScopeNode* node);
	ScopeNodeHistory(ScopeNodeHistory* original);
	~ScopeNodeHistory();
};

#endif /* SCOPE_NODE_H */