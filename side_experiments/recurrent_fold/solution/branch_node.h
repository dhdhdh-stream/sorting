/**
 * Branch size always 2.
 * (So essentially saying there's no continuous generalization, with that effect instead achieved through loops.)
 */

#ifndef BRANCH_NODE_H
#define BRANCH_NODE_H

class BranchNode : public AbstractNode {
public:
	std::vector<int> branch_scope_context;
	std::vector<int> branch_node_context;
	bool branch_is_pass_through;
	StateNetwork* branch_score_network;
	// Note: eventually, ideally, right branch will have score 0.0 while other will be negative
	int branch_exit_depth;
	int branch_next_node_id;

	StateNetwork* original_score_network;
	int original_next_node_id;

	BranchNode(std::vector<int> branch_scope_context,
			   std::vector<int> branch_node_context,
			   bool branch_is_pass_through,
			   StateNetwork* branch_score_network,
			   int branch_exit_depth,
			   int branch_next_node_id,
			   int branch_num_travelled,
			   StateNetwork* original_score_network,
			   int original_next_node_id);
	~BranchNode();

	void activate(std::vector<double>& local_state_vals,
				  std::vector<double>& input_vals,
				  std::vector<std::vector<double>>& flat_vals,
				  double& predicted_score,
				  double& scale_factor
				  std::vector<int>& scope_context,
				  std::vector<int>& node_context,
				  int& exit_depth,
				  int& exit_node_id,
				  BranchNodeHistory* history);
	void backprop(std::vector<double>& local_state_errors,
				  std::vector<double>& input_errors,
				  double target_val,
				  double& predicted_score,
				  double& scale_factor,
				  RunHelper& run_helper,
				  BranchNodeHistory* history);

};

class BranchNodeHistory : public AbstractNodeHistory {
public:
	bool is_branch;
	StateNetworkHistory* score_network_history;
	double score_network_update;

	BranchNodeHistory(BranchNode* node);
	~BranchNodeHistory();
};

#endif /* BRANCH_NODE_H */