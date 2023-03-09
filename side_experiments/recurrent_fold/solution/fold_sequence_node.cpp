#include "fold_sequence_node.h"

using namespace std;

FoldSequenceNode::FoldSequenceNode(Fold* fold,
								   int next_node_id) {
	this->type = NODE_TYPE_FOLD_SEQUENCE;

	this->fold = fold;
	this->next_node_id = next_node_id;
}

FoldSequenceNode::~FoldSequenceNode() {
	if (this->fold != NULL) {
		delete this->fold;
	}
}

void FoldSequenceNode::activate(FoldHistory* fold_history,
								vector<double>& local_state_vals,
								vector<double>& input_vals,
								vector<vector<double>>& flat_vals,
								double& predicted_score,
								double& scale_factor,
								double& sum_impact,
								RunHelper& run_helper,
								FoldSequenceNodeHistory* history) {
	this->fold->sequence_activate(local_state_vals,
								  input_vals,
								  flat_vals,
								  predicted_score,
								  scale_factor,
								  sum_impact,
								  run_helper,
								  fold_history);

	history->fold_history = fold_history;
}

void FoldSequenceNode::backprop(vector<double>& local_state_errors,
								vector<double>& input_errors,
								double target_val,
								double final_misguess,
								double final_sum_impact,
								double& predicted_score,
								double& scale_factor,
								RunHelper& run_helper,
								FoldSequenceNodeHistory* history) {
	this->fold->backprop(local_state_errors,
						 input_errors,
						 target_val,
						 final_misguess,
						 final_sum_impact,
						 predicted_score,
						 scale_factor,
						 run_helper,
						 history->fold_history);
}

FoldSequenceNodeHistory::FoldSequenceNodeHistory(FoldSequenceNode* node) {
	this->node = node;
}

FoldSequenceNodeHistory::~FoldSequenceNodeHistory() {
	delete this->fold_history;
}
