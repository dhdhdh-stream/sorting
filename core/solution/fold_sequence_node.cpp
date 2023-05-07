#include "fold_sequence_node.h"

#include <iostream>

using namespace std;

FoldSequenceNode::FoldSequenceNode(int next_node_id) {
	this->type = NODE_TYPE_FOLD_SEQUENCE;

	this->next_node_id = next_node_id;
}

FoldSequenceNode::FoldSequenceNode(ifstream& input_file,
								   int scope_id,
								   int scope_index) {
	this->type = NODE_TYPE_FOLD_SEQUENCE;

	string next_node_id_line;
	getline(input_file, next_node_id_line);
	this->next_node_id = stoi(next_node_id_line);
}

FoldSequenceNode::~FoldSequenceNode() {
	// delete fold if needed in FoldScoreNode
}

void FoldSequenceNode::activate(FoldHistory* fold_history,
								Problem& problem,
								vector<double>& state_vals,
								std::vector<bool>& states_initialized,
								double& predicted_score,
								double& scale_factor,
								double& sum_impact,
								RunHelper& run_helper,
								FoldSequenceNodeHistory* history) {
	fold_history->fold->sequence_activate(problem,
										  state_vals,
										  states_initialized,
										  predicted_score,
										  scale_factor,
										  sum_impact,
										  run_helper,
										  fold_history);

	history->fold_history = fold_history;
}

void FoldSequenceNode::backprop(vector<double>& state_errors,
								std::vector<bool>& states_initialized,
								double target_val,
								double final_diff,
								double final_misguess,
								double final_sum_impact,
								double& predicted_score,
								double& scale_factor,
								double& scale_factor_error,
								RunHelper& run_helper,
								FoldSequenceNodeHistory* history) {
	history->fold_history->fold->sequence_backprop(state_errors,
												   states_initialized,
												   target_val,
												   final_diff,
												   final_misguess,
												   final_sum_impact,
												   predicted_score,
												   scale_factor,
												   scale_factor_error,
												   run_helper,
												   history->fold_history);
}

void FoldSequenceNode::save(ofstream& output_file,
							int scope_id,
							int scope_index) {
	output_file << this->next_node_id << endl;
}

void FoldSequenceNode::save_for_display(ofstream& output_file) {
	output_file << this->next_node_id << endl;
}

FoldSequenceNodeHistory::FoldSequenceNodeHistory(FoldSequenceNode* node,
												 int scope_index) {
	this->node = node;
	this->scope_index = scope_index;
}

FoldSequenceNodeHistory::~FoldSequenceNodeHistory() {
	// delete fold_history if needed in FoldScoreNodeHistory
}
