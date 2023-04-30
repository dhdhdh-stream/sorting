#ifndef FOLD_SCORE_NODE_H
#define FOLD_SCORE_NODE_H

#include <vector>

#include "abstract_node.h"
#include "fold.h"
#include "scope.h"
#include "state_network.h"

class FoldScoreNodeHistory;
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
	FoldScoreNode(std::ifstream& input_file,
				  int scope_id,
				  int scope_index);
	~FoldScoreNode();

	void activate(std::vector<double>& state_vals,
				  double& predicted_score,
				  double& scale_factor,
				  std::vector<int>& scope_context,
				  std::vector<int>& node_context,
				  std::vector<ScopeHistory*>& context_histories,
				  int& exit_depth,
				  int& exit_node_id,
				  FoldHistory*& exit_fold_history,
				  RunHelper& run_helper,
				  FoldScoreNodeHistory* history);
	void backprop(std::vector<double>& state_errors,
				  double target_val,
				  double& predicted_score,
				  double& scale_factor,
				  RunHelper& run_helper,
				  FoldScoreNodeHistory* history);

	void save(std::ofstream& output_file,
			  int scope_id,
			  int scope_index);
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
};

#endif /* FOLD_SCORE_NODE_H */