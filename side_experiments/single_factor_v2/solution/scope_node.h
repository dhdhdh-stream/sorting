#ifndef SCOPE_NODE_H
#define SCOPE_NODE_H

const int INPUT_TYPE_STATE = 0;
const int INPUT_TYPE_CONSTANT = 1;

class ScopeNode : public AbstractNode {
public:
	int inner_scope_id;

	std::vector<int> starting_node_ids;

	std::vector<int> input_types;
	std::vector<int> input_inner_layers;
	std::vector<int> input_inner_ids;
	std::vector<int> input_outer_ids;
	std::vector<double> input_init_vals;
	/**
	 * - don't worry about reversing signs
	 *   - can only be an issue with perfect XORs
	 *     - otherwise, can align state polarity when constructing
	 *   - makes it difficult to squash sequences into new scopes
	 */

	std::vector<int> output_inner_ids;
	std::vector<int> output_outer_ids;

	int next_node_id;



};

class ScopeNodeHistory : public AbstractNodeHistory {
public:

	bool is_halfway;

};

#endif /* SCOPE_NODE_H */