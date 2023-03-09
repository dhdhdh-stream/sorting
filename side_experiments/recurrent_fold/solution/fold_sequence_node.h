#ifndef FOLD_SEQUENCE_NODE_H
#define FOLD_SEQUENCE_NODE_H

class FoldSequenceNode : public AbstractNode {
public:
	Fold* fold;

	int next_node_id;

	FoldSequenceNode(Fold* fold,
					 int next_node_id);
	~FoldSequenceNode();

	void activate(FoldHistory* fold_history,
				  std::vector<double>& local_state_vals,
				  std::vector<double>& input_vals,
				  std::vector<std::vector<double>>& flat_vals,
				  double& predicted_score,
				  double& scale_factor,
				  double& sum_impact,
				  RunHelper& run_helper,
				  FoldSequenceNodeHistory* history);
	void backprop(std::vector<double>& local_state_errors,
				  std::vector<double>& input_errors,
				  double target_val,
				  double final_misguess,
				  double final_sum_impact,
				  double& predicted_score,
				  double& scale_factor,
				  RunHelper& run_helper,
				  FoldSequenceNodeHistory* history);

};

class FoldSequenceNodeHistory : public AbstractNodeHistory {
public:
	FoldHistory* fold_history;

	FoldSequenceNodeHistory(FoldSequenceNode* node);
	~FoldSequenceNodeHistory();
};

#endif /* FOLD_SEQUENCE_NODE_H */