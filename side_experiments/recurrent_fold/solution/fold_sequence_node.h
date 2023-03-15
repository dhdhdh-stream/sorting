#ifndef FOLD_SEQUENCE_NODE_H
#define FOLD_SEQUENCE_NODE_H

#include <vector>

#include "abstract_node.h"
#include "fold.h"
#include "run_helper.h"

class FoldSequenceNodeHistory;
class FoldSequenceNode : public AbstractNode {
public:
	int next_node_id;

	FoldSequenceNode(int next_node_id);
	FoldSequenceNode(std::ifstream& input_file,
					 int scope_id,
					 int scope_index);
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

	void save(std::ofstream& output_file,
			  int scope_id,
			  int scope_index);
};

class FoldSequenceNodeHistory : public AbstractNodeHistory {
public:
	FoldHistory* fold_history;

	FoldSequenceNodeHistory(FoldSequenceNode* node,
							int scope_index);
	~FoldSequenceNodeHistory();
};

#endif /* FOLD_SEQUENCE_NODE_H */