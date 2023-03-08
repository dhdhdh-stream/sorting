#ifndef SCOPE_NODE_H
#define SCOPE_NODE_H

class ScopeNode : public AbstractNode {
public:
	// if any, pre_state_network target always is local
	std::vector<int> pre_state_network_target_indexes;
	std::vector<StateNetwork*> pre_state_networks;

	int inner_scope_id;
	std::vector<bool> inner_input_is_local;
	std::vector<int> inner_input_indexes;
	// Scale* scope_scale_mod;

	std::vector<bool> post_state_network_target_is_local;
	std::vector<int> post_state_network_target_indexes;
	std::vector<StateNetwork*> post_state_networks;

	StateNetwork* score_network;

	int next_node_id;	// TODO: set when adding to Scope?
	// may go elsewhere due to early exit, but handled from inner

	double average_score;
	double score_variance;
	double average_misguess;
	double misguess_variance;

	double average_impact;
	double average_sum_impact;

	// TODO: add batch surprise and seeding

	std::vector<int> explore_scope_context;
	std::vector<int> explore_node_context;
	// TODO: explore_action_sequences
	int explore_exit_depth;
	int explore_next_node_id;
	Fold* explore_fold;

	ScopeNode(std::vector<bool> pre_state_network_target_is_local,
			  std::vector<int> pre_state_network_target_indexes,
			  std::vector<StateNetwork*> pre_state_networks,
			  int inner_scope_id,
			  std::vector<bool> inner_input_is_local,
			  std::vector<int> inner_input_indexes,
			  std::vector<bool> post_state_network_target_is_local,
			  std::vector<int> post_state_network_target_indexes,
			  std::vector<StateNetwork*> post_state_networks,
			  StateNetwork* score_network);
	~ScopeNode();

	void activate(std::vector<double>& local_state_vals,
				  std::vector<double>& input_vals,
				  std::vector<std::vector<double>>& flat_vals,
				  double& predicted_score,
				  double& scale_factor,
				  double& sum_impact,
				  std::vector<int>& scope_context,
				  std::vector<int>& node_context,
				  std::vector<int>& context_iter,
				  std::vector<ContextHistory*>& context_histories,
				  int& early_exit_depth,
				  int& early_exit_node_id,
				  FoldHistory*& early_exit_fold_history,
				  int& explore_exit_depth,
				  int& explore_exit_node_id,
				  FoldHistory*& explore_exit_fold_history,
				  RunHelper& run_helper,
				  ScopeNodeHistory* history);
	void backprop(std::vector<double>& local_state_errors,
				  std::vector<double>& input_errors,
				  double target_val,
				  double final_misguess,
				  double final_sum_impact,
				  double& predicted_score,
				  double& scale_factor,
				  RunHelper& run_helper,
				  ScopeNodeHistory* history);

};

class ScopeNodeHistory : public AbstractNodeHistory {
public:
	std::vector<StateNetworkHistory*> pre_state_network_histories;

	ScopeHistory* inner_scope_history;

	bool inner_is_early_exit;
	std::vector<StateNetworkHistory*> post_state_network_histories;
	StateNetworkHistory* score_network_history;
	double score_network_update;

	ScopeNodeHistory(ScopeNode* node);
	~ScopeNodeHistory();
};

#endif /* SCOPE_NODE_H */