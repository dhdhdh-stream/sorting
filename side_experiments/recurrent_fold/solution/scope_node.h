#ifndef SCOPE_NODE_H
#define SCOPE_NODE_H

class ScopeNode : public AbstractNode {
public:
	std::vector<bool> pre_state_network_target_is_local;
	std::vector<int> pre_state_network_target_indexes;
	std::vector<StateNetwork*> pre_state_networks;

	Scope* inner_scope;
	std::vector<bool> inner_input_is_local;
	std::vector<int> inner_input_indexes;
	// std::vector<Network*> scope_scale_mod;

	std::vector<bool> post_state_network_target_is_local;
	std::vector<int> post_state_network_target_indexes;
	std::vector<StateNetwork*> post_state_networks;

	ScoreNetwork* score_network;

	int next_node_id;	// TODO: set when adding to Scope?
	// may go elsewhere due to early exit, but no need to track (and don't explore branch there)

	// TODO: don't explore inner unless sufficient weight

	ScopeNode(Scope* parent,
			  Scope* inner_scope,
			  std::vector<bool> inner_input_is_local,
			  std::vector<int> inner_input_indexes,
			  bool has_score_network,
			  Network* score_network);
	~ScopeNode();

	int activate(std::vector<double>& input_vals,
				 std::vector<double>& local_state_vals,
				 std::vector<std::vector<double>>& flat_vals,
				 double& predicted_score,
				 int& early_exit_count,
				 int& early_exit_index);

	void explore_score(std::vector<double>& input_vals,
					   std::vector<double>& local_state_vals,
					   double& predicted_score,
					   double& scale_factor,
					   std::vector<int>& context_iter,
					   double& new_outer_state_val,
					   RunHelper& run_helper);
};

#endif /* SCOPE_NODE_H */