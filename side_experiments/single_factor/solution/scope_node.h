#ifndef SCOPE_NODE_H
#define SCOPE_NODE_H

class ScopeNode : public AbstractNode {
public:
	int inner_scope_id;

	std::vector<int> starting_node_ids;

	std::vector<int> input_indexes;
	std::vector<int> input_target_layers;
	std::vector<int> input_target_indexes;
	std::vector<bool> input_is_copy;

	Scale* scope_scale_mod;

	int next_node_id;



	void seed_activate(ScopeNodeHistory* seed_history,
					   ScopeNodeHistory* history);

};

class ScopeNodeHistory : public AbstractNodeHistory {
public:
	ScopeHistory* inner_scope_history;

	ScopeNodeHistory(ScopeNode* node);
	ScopeNodeHistory(ScopeNodeHistory* original);	// deep copy for seed
	~ScopeNodeHistory();
};

#endif /* SCOPE_NODE_H */