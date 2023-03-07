#ifndef ACTION_NODE_H
#define ACTION_NODE_H

class ActionNode : public AbstractNode {
public:
	std::vector<bool> state_network_target_is_local;
	std::vector<int> state_network_target_indexes;
	std::vector<StateNetwork*> state_networks;

	StateNetwork* score_network;

	int next_node_id;	// TODO: set when adding to Scope?

	double average_score;
	double score_variance;
	double average_misguess;
	double misguess_variance;

	double average_impact;

	// TODO: add batch surprise and seeding

	std::vector<int> explore_scope_context;
	std::vector<int> explore_node_context;
	// TODO: explore_action_sequences
	int explore_exit_depth;
	int explore_next_node_id;
	Fold* explore_fold;

	ActionNode(Scope* parent,
			   std::vector<bool> state_network_target_is_local,
			   std::vector<int> state_network_target_indexes,
			   std::vector<StateNetwork*> state_networks,
			   StateNetwork* score_network);
	~ActionNode();

	void explore_on_path_activate(std::vector<double>& local_state_vals,
								  std::vector<double>& input_vals,
								  std::vector<std::vector<double>>& flat_vals,
								  double& predicted_score,
								  double& scale_factor,
								  RunHelper& run_helper,
								  ActionNodeHistory* history);
	void explore_on_path_backprop(std::vector<double>& local_state_errors,
								  std::vector<double>& input_errors,
								  double target_val,
								  double& predicted_score,
								  double& scale_factor,
								  ActionNodeHistory* history);



	void update_activate(std::vector<double>& local_state_vals,
						 std::vector<double>& input_vals,
						 std::vector<std::vector<double>>& flat_vals,
						 double& predicted_score,
						 double& scale_factor,
						 double& sum_impact,
						 double& explore_weight_scale_factor,
						 ActionNodeHistory* history);
	void update_backprop(double target_val,
						 double final_misguess,
						 double& predicted_score,
						 double& scale_factor,
						 double sum_impact_error,
						 double& explore_weight_scale_factor_error,
						 ActionNodeHistory* history);


};

class ActionNodeHistory : public AbstractNodeHistory {
public:
	std::vector<StateNetworkHistory*> state_network_histories;
	StateNetworkHistory* score_network_history;
	double score_network_update;

	ActionNodeHistory(ActionNode* node);
	~ActionNodeHistory();
};

#endif /* ACTION_NODE_H */