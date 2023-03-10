#ifndef FOLD_SCORE_NODE_H
#define FOLD_SCORE_NODE_H

class FoldScoreNode : public AbstractNode {
public:
	StateNetwork* existing_score_network;
	int existing_next_node_id;

	Fold* fold;

	bool fold_is_pass_through;
	std::vector<int> fold_scope_context;
	std::vector<int> fold_node_context;
	int fold_num_travelled;	// if there's recursion, scores may be inaccurate, so ease in to new branch

	int fold_exit_depth;
	int fold_next_node_id;

	FoldScoreNode(StateNetwork* existing_score_network,
				  int existing_next_node_id,
				  Fold* fold,
				  bool fold_is_pass_through,
				  std::vector<int> fold_scope_context,
				  std::vector<int> fold_node_context,
				  int fold_exit_depth,
				  int fold_next_node_id);
	~FoldScoreNode();

	void activate(std::vector<double>& local_state_vals,
				  std::vector<double>& input_vals,
				  double& predicted_score,
				  double& scale_factor,
				  std::vector<int>& scope_context,
				  std::vector<int>& node_context,
				  std::vector<int>& context_iter,
				  std::vector<ContextHistory*>& context_histories,
				  int& exit_depth,
				  int& exit_node_id,
				  FoldHistory*& exit_fold_history,
				  RunHelper& run_helper,
				  FoldScoreNodeHistory* history);
	void backprop(std::vector<double>& local_state_errors,
				  std::vector<double>& input_errors,
				  double target_val,
				  double& predicted_score,
				  double& scale_factor,
				  RunHelper& run_helper,
				  FoldScoreNodeHistory* history);

};

class FoldScoreNodeHistory : public AbstractNodeHistory {
public:
	bool is_existing;
	StateNetworkHistory* existing_score_network_history;
	double existing_score_network_update;

	// FoldHistory saved and processed by FoldSequenceNode

	FoldScoreNodeHistory(FoldScoreNode* node,
						 int scope_index);
	~FoldScoreNodeHistory();
}

#endif /* FOLD_SCORE_NODE_H */