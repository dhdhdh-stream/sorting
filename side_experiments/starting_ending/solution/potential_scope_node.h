#ifndef POTENTIAL_SCOPE_NODE_H
#define POTENTIAL_SCOPE_NODE_H

const int OUTER_TYPE_INPUT = 0;
const int OUTER_TYPE_LOCAL = 1;
const int OUTER_TYPE_TEMP = 2;

class PotentialScopeNode {
public:
	std::vector<int> input_types;
	std::vector<int> input_inner_indexes;
	/**
	 * - negative indexing
	 */
	std::vector<int> input_scope_depths;
	std::vector<int> input_outer_types;
	std::vector<void*> input_outer_indexes;
	std::vector<double> input_init_vals;

	std::vector<int> output_inner_indexes;
	std::vector<int> output_scope_depths;
	std::vector<int> output_outer_types;
	std::vector<void*> output_outer_indexes;

	Scope* scope;

	ScopeNode* scope_node_placeholder;

	
};

#endif /* POTENTIAL_SCOPE_NODE_H */