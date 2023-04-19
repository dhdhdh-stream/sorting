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
								vector<double>& state_vals,
								std::vector<bool>& states_initialized,
								vector<vector<double>>& flat_vals,
								double& predicted_score,
								double& scale_factor,
								double& sum_impact,
								RunHelper& run_helper,
								FoldSequenceNodeHistory* history) {
	fold_history->fold->sequence_activate(state_vals,
										  states_initialized
										  flat_vals,
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
								double final_misguess,
								double final_sum_impact,
								double& predicted_score,
								double& scale_factor,
								RunHelper& run_helper,
								FoldSequenceNodeHistory* history) {
	history->fold_history->fold->backprop(state_errors,
										  states_initialized,
										  target_val,
										  final_misguess,
										  final_sum_impact,
										  predicted_score,
										  scale_factor,
										  run_helper,
										  history->fold_history);
}

void FoldSequenceNode::save(ofstream& output_file,
							int scope_id,
							int scope_index) {
	output_file << this->next_node_id << endl;
}

FoldSequenceNodeHistory::FoldSequenceNodeHistory(FoldSequenceNode* node,
												 int scope_index) {
	this->node = node;
	this->scope_index = scope_index;
}

FoldSequenceNodeHistory::~FoldSequenceNodeHistory() {
	delete this->fold_history;
}
