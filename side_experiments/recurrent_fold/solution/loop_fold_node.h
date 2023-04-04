#ifndef LOOP_FOLD_NODE_H
#define LOOP_FOLD_NODE_H

#include <vector>

#include "abstract_node.h"
#include "loop_fold.h"
#include "run_helper.h"

class LoopFoldNodeHistory;
class LoopFoldNode: public AbstractNode {
public:
	LoopFold* loop_fold;

	std::vector<int> fold_scope_context;
	std::vector<int> fold_node_context;

	int next_node_id;

	LoopFoldNode(LoopFold* loop_fold,
				 std::vector<int> fold_scope_context,
				 std::vector<int> fold_node_context,
				 int next_node_id);
	LoopFoldNode(std::ifstream& input_file,
				 int scope_id,
				 int scope_index);
	~LoopFoldNode();

	void activate(std::vector<double>& local_state_vals,
				  std::vector<double>& input_vals,
				  std::vector<std::vector<double>>& flat_vals,
				  double& predicted_score,
				  double& scale_factor,
				  double& sum_impact,
				  std::vector<int>& scope_context,
				  std::vector<int>& node_context,
				  std::vector<ScopeHistory*>& context_histories,
				  RunHelper& run_helper,
				  LoopFoldNodeHistory* history);
	void backprop(std::vector<double>& local_state_errors,
				  std::vector<double>& input_errors,
				  double target_val,
				  double final_misguess,
				  double final_sum_impact,
				  double& predicted_score,
				  double& scale_factor,
				  RunHelper& run_helper,
				  LoopFoldNodeHistory* history);

	void save(std::ofstream& output_file,
			  int scope_id,
			  int scope_index);
};

class LoopFoldNodeHistory : public AbstractNodeHistory {
public:
	LoopFoldHistory* loop_fold_history;

	LoopFoldNodeHistory(LoopFoldNode* node,
						int scope_index);
	~LoopFoldNodeHistory();
};

#endif /* LOOP_FOLD_NODE_H */