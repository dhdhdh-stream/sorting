#include "fold_score_node.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "utilities.h"

using namespace std;

FoldScoreNode::FoldScoreNode(StateNetwork* existing_score_network,
							 int existing_next_node_id,
							 Fold* fold,
							 bool fold_is_pass_through,
							 vector<int> fold_scope_context,
							 vector<int> fold_node_context,
							 int fold_exit_depth,
							 int fold_next_node_id) {
	this->type = NODE_TYPE_FOLD_SCORE;

	this->existing_score_network = existing_score_network;
	this->existing_next_node_id = existing_next_node_id;

	this->fold = fold;

	this->fold_is_pass_through = fold_is_pass_through;
	this->fold_scope_context = fold_scope_context;
	this->fold_node_context = fold_node_context;
	this->fold_num_travelled = 0;

	this->fold_exit_depth = fold_exit_depth;
	this->fold_next_node_id = fold_next_node_id;
}

FoldScoreNode::FoldScoreNode(ifstream& input_file,
							 int scope_id,
							 int scope_index) {
	this->type = NODE_TYPE_FOLD_SCORE;

	ifstream existing_score_network_save_file;
	existing_score_network_save_file.open("saves/nns/" + to_string(scope_id) + "_" + to_string(scope_index) + "_existing_score.txt");
	this->existing_score_network = new StateNetwork(existing_score_network_save_file);
	existing_score_network_save_file.close();

	string existing_next_node_id_line;
	getline(input_file, existing_next_node_id_line);
	this->existing_next_node_id = stoi(existing_next_node_id_line);

	ifstream fold_save_file;
	fold_save_file.open("saves/fold_" + to_string(scope_id) + "_" + to_string(scope_index) + ".txt");
	this->fold = new Fold(fold_save_file,
						  scope_id,
						  scope_index);
	fold_save_file.close();

	string is_pass_through_line;
	getline(input_file, is_pass_through_line);
	this->fold_is_pass_through = stoi(is_pass_through_line);

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

	string num_travelled_line;
	getline(input_file, num_travelled_line);
	this->fold_num_travelled = stoi(num_travelled_line);

	string fold_exit_depth_line;
	getline(input_file, fold_exit_depth_line);
	this->fold_exit_depth = stoi(fold_exit_depth_line);

	string fold_next_node_id_line;
	getline(input_file, fold_next_node_id_line);
	this->fold_next_node_id = stoi(fold_next_node_id_line);
}

FoldScoreNode::~FoldScoreNode() {
	if (this->existing_score_network != NULL) {
		delete this->existing_score_network;
	}

	if (this->fold != NULL) {
		delete this->fold;
	}
}

void FoldScoreNode::activate(vector<double>& state_vals,
							 double& predicted_score,
							 double& scale_factor,
							 vector<int>& scope_context,
							 vector<int>& node_context,
							 vector<ScopeHistory*>& context_histories,
							 int& exit_depth,
							 int& exit_node_id,
							 FoldHistory*& exit_fold_history,
							 RunHelper& run_helper,
							 FoldScoreNodeHistory* history) {
	bool fold_avail = true;
	if (this->fold_num_travelled < 100000) {
		if (randuni() > (double)this->fold_num_travelled/100000) {
			fold_avail = false;
		}
		this->fold_num_travelled++;
	}

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

	if (fold_avail && matches_context) {
		FoldHistory* fold_history = new FoldHistory(this->fold);
		this->fold->score_activate(state_vals,
								   predicted_score,
								   scale_factor,
								   context_histories,
								   run_helper,
								   fold_history);
		double fold_score = scale_factor*fold_history->starting_score_update;

		if (this->fold_is_pass_through) {
			history->is_existing = false;

			history->fold_history = fold_history;

			predicted_score += fold_score;

			exit_depth = this->fold_exit_depth;
			exit_node_id = this->fold_next_node_id;
			exit_fold_history = fold_history;
		} else {
			StateNetworkHistory* existing_network_history = new StateNetworkHistory(this->existing_score_network);
			this->existing_score_network->activate(state_vals,
												   existing_network_history);
			double existing_score = scale_factor*this->existing_score_network->output->acti_vals[0];

			if (fold_score > existing_score) {
				delete existing_network_history;

				history->is_existing = false;

				history->fold_history = fold_history;

				predicted_score += fold_score;

				exit_depth = this->fold_exit_depth;
				exit_node_id = this->fold_next_node_id;
				exit_fold_history = fold_history;

				global_debug_flag = true;
			} else {
				delete fold_history;

				history->is_existing = true;
				history->existing_score_network_history = existing_network_history;
				history->existing_score_network_update = this->existing_score_network->output->acti_vals[0];

				predicted_score += existing_score;

				exit_depth = 0;
				exit_node_id = this->existing_next_node_id;
				exit_fold_history = NULL;
			}
		}
	} else {
		StateNetworkHistory* existing_network_history = new StateNetworkHistory(this->existing_score_network);
		this->existing_score_network->activate(state_vals,
											   existing_network_history);

		history->is_existing = true;
		history->existing_score_network_history = existing_network_history;
		history->existing_score_network_update = this->existing_score_network->output->acti_vals[0];

		predicted_score += scale_factor*this->existing_score_network->output->acti_vals[0];

		exit_depth = 0;
		exit_node_id = this->existing_next_node_id;
		exit_fold_history = NULL;
	}
}

void FoldScoreNode::backprop(vector<double>& state_errors,
							 double target_val,
							 double& predicted_score,
							 double& scale_factor,
							 double& scale_factor_error,
							 RunHelper& run_helper,
							 FoldScoreNodeHistory* history) {
	if (history->is_existing) {
		if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN) {
			double predicted_score_error = target_val - predicted_score;
			this->existing_score_network->backprop_errors_with_no_weight_change(
				scale_factor*predicted_score_error,
				state_errors,
				history->existing_score_network_history);

			predicted_score -= scale_factor*history->existing_score_network_update;
		} else if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE) {
			double predicted_score_error = target_val - predicted_score;

			scale_factor_error += history->existing_score_network_update*predicted_score_error;

			this->existing_score_network->backprop_weights_with_no_error_signal(
				scale_factor*predicted_score_error,
				0.002,
				history->existing_score_network_history);

			predicted_score -= scale_factor*history->existing_score_network_update;
		}
	} else {
		this->fold->score_backprop(state_errors,
								   target_val,
								   predicted_score,
								   scale_factor,
								   scale_factor_error,
								   run_helper,
								   history->fold_history);
	}
}

void FoldScoreNode::save(ofstream& output_file,
						 int scope_id,
						 int scope_index) {
	ofstream existing_score_network_save_file;
	existing_score_network_save_file.open("saves/nns/" + to_string(scope_id) + "_" + to_string(scope_index) + "_existing_score.txt");
	this->existing_score_network->save(existing_score_network_save_file);
	existing_score_network_save_file.close();

	output_file << this->existing_next_node_id << endl;

	ofstream fold_save_file;
	fold_save_file.open("saves/fold_" + to_string(scope_id) + "_" + to_string(scope_index) + ".txt");
	this->fold->save(fold_save_file,
					 scope_id,
					 scope_index);
	fold_save_file.close();

	output_file << this->fold_is_pass_through << endl;
	output_file << this->fold_scope_context.size() << endl;
	for (int c_index = 0; c_index < (int)this->fold_scope_context.size(); c_index++) {
		output_file << this->fold_scope_context[c_index] << endl;
		output_file << this->fold_node_context[c_index] << endl;
	}
	output_file << this->fold_num_travelled << endl;

	output_file << this->fold_exit_depth << endl;
	output_file << this->fold_next_node_id << endl;
}

void FoldScoreNode::save_for_display(ofstream& output_file) {
	output_file << this->existing_next_node_id << endl;

	output_file << this->fold_scope_context[this->fold_exit_depth] << endl;
	output_file << this->fold_next_node_id << endl;
}

FoldScoreNodeHistory::FoldScoreNodeHistory(FoldScoreNode* node,
										   int scope_index) {
	this->node = node;
	this->scope_index = scope_index;

	this->existing_score_network_history = NULL;
	this->fold_history = NULL;
}

FoldScoreNodeHistory::~FoldScoreNodeHistory() {
	if (this->existing_score_network_history != NULL) {
		delete this->existing_score_network_history;
	}

	if (this->fold_history != NULL) {
		delete this->fold_history;
	}
}
