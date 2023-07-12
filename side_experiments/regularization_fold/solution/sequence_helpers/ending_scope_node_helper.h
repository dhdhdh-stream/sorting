#ifndef ENDING_SCOPE_NODE_HELPER_H
#define ENDING_SCOPE_NODE_HELPER_H

class EndingScopeNodeActivateHelper {
public:
	ScopeNode* scope_node;

	vector<double>* curr_state_vals;
	vector<bool>* curr_states_initialized;

	bool is_halfway;
	int furthest_matching_layer;

	vector<vector<double>> inner_state_vals;
	vector<vector<double>*> starting_state_vals_save;

	EndingScopeNodeActivateHelper(ScopeNode* scope_node);

	void forward(std::vector<int>& next_starting_node_ids,
				 std::vector<std::vector<double>*>& next_starting_state_vals,
				 std::vector<std::vector<bool>>& next_starting_states_initialized,
				 std::vector<ForwardContextLayer>& context,
				 RunHelper& run_helper);
	void backward(std::vector<ForwardContextLayer>& context,
				  RunHelper& run_helper);
};

class EndingScopeNodeBackpropHelper {
public:
	ScopeNode* scope_node;

	vector<double>* curr_state_errors;

	bool is_halfway;
	int furthest_matching_layer;

	vector<vector<double>> inner_state_errors;
	vector<vector<double>*> starting_state_errors_save;

	EndingScopeNodeBackpropHelper(ScopeNode* scope_node);

	void backward(std::vector<int>& next_starting_node_ids,
				  std::vector<std::vector<double>*>& next_starting_state_errors,
				  std::vector<BackwardContextLayer>& context,
				  RunHelper& run_helper);
	void forward(std::vector<BackwardContextLayer>& context,
				 RunHelper& run_helper);
};

#endif /* ENDING_SCOPE_NODE_HELPER_H */