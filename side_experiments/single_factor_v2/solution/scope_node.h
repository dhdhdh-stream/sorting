#ifndef SCOPE_NODE_H
#define SCOPE_NODE_H

class ScopeNode : public AbstractNode {
public:
	int inner_scope_id;

	std::vector<int> starting_node_ids;

	std::vector<int> input_types;
	std::vector<int> input_target_layers;
	std::vector<int> input_target_ids;
	std::vector<int> input_ids;
	std::vector<bool> input_reverse_sign_front;
	std::vector<bool> input_reverse_sign_back;
	std::vector<double> input_init_vals;

	int next_node_id;



};

class ScopeNodeHistory : public AbstractNodeHistory {
public:

	bool is_halfway;

};

#endif /* SCOPE_NODE_H */