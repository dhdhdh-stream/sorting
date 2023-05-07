#include "loop_fold_node.h"

#include <iostream>

using namespace std;

LoopFoldNode::LoopFoldNode(LoopFold* loop_fold,
						   std::vector<int> fold_scope_context,
						   std::vector<int> fold_node_context,
						   int next_node_id) {
	this->type = NODE_TYPE_LOOP_FOLD;

	this->loop_fold = loop_fold;
	this->fold_scope_context = fold_scope_context;
	this->fold_node_context = fold_node_context;
	this->next_node_id = next_node_id;
}

LoopFoldNode::LoopFoldNode(ifstream& input_file,
						   int scope_id,
						   int scope_index) {
	this->type = NODE_TYPE_LOOP_FOLD;

	ifstream loop_fold_save_file;
	loop_fold_save_file.open("saves/loop_fold_" + to_string(scope_id) + "_" + to_string(scope_index) + ".txt");
	this->loop_fold = new LoopFold(loop_fold_save_file,
								   scope_id,
								   scope_index);
	loop_fold_save_file.close();

	string context_size_line;
	getline(input_file, context_size_line);
	int context_size = stoi(context_size_line);
	for (int c_index = 0; c_index < context_size; c_index++) {
		string scope_context_line;
		getline(input_file, scope_context_line);
		this->fold_scope_context.push_back(stoi(scope_context_line));

		string node_context_line;
		getline(input_file, node_context_line);
		this->fold_node_context.push_back(stoi(node_context_line));
	}

	string next_node_id_line;
	getline(input_file, next_node_id_line);
	this->next_node_id = stoi(next_node_id_line);
}

LoopFoldNode::~LoopFoldNode() {
	if (this->loop_fold != NULL) {
		delete this->loop_fold;
	}
}

void LoopFoldNode::activate(Problem& problem,
							vector<double>& state_vals,
							vector<bool>& states_initialized,
							double& predicted_score,
							double& scale_factor,
							double& sum_impact,
							vector<int>& scope_context,
							vector<int>& node_context,
							vector<ScopeHistory*>& context_histories,
							RunHelper& run_helper,
							LoopFoldNodeHistory* history) {
	bool matches_context = true;
	if (this->fold_scope_context.size() > scope_context.size()) {
		matches_context = false;
	} else {
		// special case first scope context
		if (this->fold_scope_context[0] != scope_context.back()) {
			matches_context = false;
		} else {
			for (int c_index = 1; c_index < (int)this->fold_scope_context.size(); c_index++) {
				if (this->fold_scope_context[c_index] != scope_context[scope_context.size()-1-c_index]
						|| this->fold_node_context[c_index] != node_context[node_context.size()-1-c_index]) {
					matches_context = false;
					break;
				}
			}
		}
	}

	if (matches_context) {
		LoopFoldHistory* loop_fold_history = new LoopFoldHistory(this->loop_fold);
		this->loop_fold->activate(problem,
								  state_vals,
								  states_initialized,
								  predicted_score,
								  scale_factor,
								  sum_impact,
								  context_histories,
								  run_helper,
								  loop_fold_history);
		history->loop_fold_history = loop_fold_history;
	}
}

void LoopFoldNode::backprop(vector<double>& state_errors,
							vector<bool>& states_initialized,
							double target_val,
							double final_diff,
							double final_misguess,
							double final_sum_impact,
							double& predicted_score,
							double& scale_factor,
							double& scale_factor_error,
							RunHelper& run_helper,
							LoopFoldNodeHistory* history) {
	if (history->loop_fold_history != NULL) {
		this->loop_fold->backprop(state_errors,
								  states_initialized,
								  target_val,
								  final_diff,
								  final_misguess,
								  final_sum_impact,
								  predicted_score,
								  scale_factor,
								  scale_factor_error,
								  run_helper,
								  history->loop_fold_history);
	}
}

void LoopFoldNode::save(ofstream& output_file,
						int scope_id,
						int scope_index) {
	ofstream loop_fold_save_file;
	loop_fold_save_file.open("saves/loop_fold_" + to_string(scope_id) + "_" + to_string(scope_index) + ".txt");
	this->loop_fold->save(loop_fold_save_file,
						  scope_id,
						  scope_index);
	loop_fold_save_file.close();

	output_file << this->fold_scope_context.size() << endl;
	for (int c_index = 0; c_index < (int)this->fold_scope_context.size(); c_index++) {
		output_file << this->fold_scope_context[c_index] << endl;
		output_file << this->fold_node_context[c_index] << endl;
	}

	output_file << this->next_node_id << endl;
}

void LoopFoldNode::save_for_display(ofstream& output_file) {
	output_file << this->next_node_id << endl;
}

LoopFoldNodeHistory::LoopFoldNodeHistory(LoopFoldNode* node,
										 int scope_index) {
	this->node = node;
	this->scope_index = scope_index;

	this->loop_fold_history = NULL;
}

LoopFoldNodeHistory::~LoopFoldNodeHistory() {
	if (this->loop_fold_history != NULL) {
		delete this->loop_fold_history;
	}
}
