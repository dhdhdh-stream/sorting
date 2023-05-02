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
				  Problem& problem,
				  std::vector<double>& state_vals,
				  std::vector<bool>& states_initialized,
				  double& predicted_score,
				  double& scale_factor,
				  double& sum_impact,
				  RunHelper& run_helper,
				  FoldSequenceNodeHistory* history);
	void backprop(std::vector<double>& state_errors,
				  std::vector<bool>& states_initialized,
				  double target_val,
				  double final_misguess,
				  double final_sum_impact,
				  double& predicted_score,
				  double& scale_factor,
				  double& scale_factor_error,
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