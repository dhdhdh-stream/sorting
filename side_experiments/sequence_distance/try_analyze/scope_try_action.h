// - really try to limit number of inputs/outputs and see how it goes

#ifndef SCOPE_TRY_ACTION_H
#define SCOPE_TRY_ACTION_H

class ScopeTryAction {
public:
	/**
	 * - original scope is scope_context[0]
	 */

	std::vector<AbstractNode*> original_nodes;
	std::vector<int> starting_original_scope_context;
	std::vector<int> starting_original_node_context;
	std::vector<int> ending_original_scope_context;
	std::vector<int> ending_original_node_context;
	std::vector<std::pair<std::pair<bool,int>, std::pair<bool,int>>> original_states;

	std::vector<int> input_types;
	std::vector<int> input_inner_indexes;
	std::vector<int> input_scope_depths;
	std::vector<int> input_outer_types;
	std::vector<void*> input_outer_indexes;
	std::vector<double> input_init_vals;
	std::vector<double> input_init_index_vals;

	std::vector<int> output_inner_indexes;
	std::vector<int> output_scope_depths;
	std::vector<int> output_outer_types;
	std::vector<void*> output_outer_indexes;

};

#endif /* SCOPE_TRY_ACTION_H */