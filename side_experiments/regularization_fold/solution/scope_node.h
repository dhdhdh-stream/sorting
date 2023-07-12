#ifndef SCOPE_NODE_H
#define SCOPE_NODE_H

class ScopeNode : public AbstractNode {
public:
	int inner_scope_id;

	std::vector<int> starting_node_ids;

	std::vector<int> input_indexes;
	std::vector<int> input_target_layers;
	std::vector<int> input_target_indexes;
	std::vector<Transformation*> input_transformations;

	Scale* scope_scale_mod;

	int next_node_id;

	void activate(std::vector<double>& flat_vals,
				  std::vector<ForwardContextLayer>& context,
				  int& inner_exit_depth,
				  int& inner_exit_node_id,
				  RunHelper& run_helper,
				  ScopeNodeHistory* history);
	void halfway_activate(std::vector<int>& starting_node_ids,
						  std::vector<std::vector<double>*>& starting_state_vals,
						  std::vector<std::vector<bool>>& starting_states_initialized,
						  std::vector<double>& flat_vals,
						  std::vector<ForwardContextLayer>& context,
						  int& inner_exit_depth,
						  int& inner_exit_node_id,
						  RunHelper& run_helper,
						  ScopeNodeHistory* history);
	void backprop(std::vector<BackwardContextLayer>& context,
				  double& scale_factor_error,
				  RunHelper& run_helper,
				  ScopeNodeHistory* history);
	void halfway_backprop(std::vector<int>& starting_node_ids,
						  std::vector<std::vector<double>*>& starting_state_errors,
						  std::vector<BackwardContextLayer>& context,
						  double& scale_factor_error,
						  RunHelper& run_helper,
						  ScopeNodeHistory* history);

};

class ScopeNodeHistory : public AbstractNodeHistory {
public:
	ScopeHistory* inner_scope_history;


};

#endif /* SCOPE_NODE_H */