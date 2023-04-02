#include "branch_node.h"

#include "constants.h"
#include "globals.h"

using namespace std;

BranchNode::BranchNode(vector<int> branch_scope_context,
					   vector<int> branch_node_context,
					   bool branch_is_pass_through,
					   StateNetwork* branch_score_network,
					   int branch_exit_depth,
					   int branch_next_node_id,
					   StateNetwork* original_score_network,
					   int original_next_node_id) {
	this->type = NODE_TYPE_BRANCH;

	this->branch_scope_context = branch_scope_context;
	this->branch_node_context = branch_node_context;
	this->branch_is_pass_through = branch_is_pass_through;
	this->branch_score_network = branch_score_network;
	this->branch_exit_depth = branch_exit_depth;
	this->branch_next_node_id = branch_next_node_id;
	this->original_score_network = original_score_network;
	this->original_next_node_id = original_next_node_id;
}

BranchNode::BranchNode(ifstream& input_file,
					   int scope_id,
					   int scope_index) {
	this->type = NODE_TYPE_BRANCH;

	string context_size_line;
	getline(input_file, context_size_line);
	int context_size = stoi(context_size_line);
	for (int c_index = 0; c_index < context_size; c_index++) {
		string scope_context_line;
		getline(input_file, scope_context_line);
		this->branch_scope_context.push_back(stoi(scope_context_line));

		string node_context_line;
		getline(input_file, node_context_line);
		this->branch_node_context.push_back(stoi(node_context_line));
	}

	string is_pass_through_line;
	getline(input_file, is_pass_through_line);
	this->branch_is_pass_through = stoi(is_pass_through_line);

	ifstream branch_score_network_save_file;
	branch_score_network_save_file.open("saves/nns/" + to_string(scope_id) + "_" + to_string(scope_index) + "_branch_score.txt");
	this->branch_score_network = new StateNetwork(branch_score_network_save_file);
	branch_score_network_save_file.close();

	string branch_exit_depth_line;
	getline(input_file, branch_exit_depth_line);
	this->branch_exit_depth = stoi(branch_exit_depth_line);

	string branch_next_node_id_line;
	getline(input_file, branch_next_node_id_line);
	this->branch_next_node_id = stoi(branch_next_node_id_line);

	ifstream original_score_network_save_file;
	original_score_network_save_file.open("saves/nns/" + to_string(scope_id) + "_" + to_string(scope_index) + "_original_score.txt");
	this->original_score_network = new StateNetwork(original_score_network_save_file);
	original_score_network_save_file.close();

	string original_next_node_id_line;
	getline(input_file, original_next_node_id_line);
	this->original_next_node_id = stoi(original_next_node_id_line);
}

BranchNode::~BranchNode() {
	delete this->branch_score_network;
	delete this->original_score_network;
}

void BranchNode::activate(vector<double>& local_state_vals,
						  vector<double>& input_vals,
						  double& predicted_score,
						  double& scale_factor,
						  vector<int>& scope_context,
						  vector<int>& node_context,
						  int& exit_depth,
						  int& exit_node_id,
						  BranchNodeHistory* history) {
	bool matches_context = true;
	if (this->branch_scope_context.size() > scope_context.size()) {
		matches_context = false;
	} else {
		// special case first scope context
		if (this->branch_scope_context[0] != scope_context.back()) {
			matches_context = false;
		} else {
			for (int c_index = 1; c_index < (int)this->branch_scope_context.size(); c_index++) {
				if (this->branch_scope_context[c_index] != scope_context[scope_context.size()-1-c_index]
						|| this->branch_node_context[c_index] != node_context[node_context.size()-1-c_index]) {
					matches_context = false;
					break;
				}
			}
		}
	}

	if (matches_context) {
		StateNetworkHistory* branch_network_history = new StateNetworkHistory(this->branch_score_network);
		this->branch_score_network->activate(local_state_vals,
											 input_vals,
											 branch_network_history);
		double branch_score = scale_factor*this->branch_score_network->output->acti_vals[0];

		if (this->branch_is_pass_through) {
			history->is_branch = true;
			history->score_network_history = branch_network_history;
			history->score_network_update = this->branch_score_network->output->acti_vals[0];

			predicted_score += branch_score;

			exit_depth = this->branch_exit_depth;
			exit_node_id = this->branch_next_node_id;
		} else {
			StateNetworkHistory* original_network_history = new StateNetworkHistory(this->original_score_network);
			this->original_score_network->activate(local_state_vals,
												   input_vals,
												   original_network_history);
			double original_score = scale_factor*this->original_score_network->output->acti_vals[0];

			if (branch_score > original_score) {
				delete original_network_history;

				history->is_branch = true;
				history->score_network_history = branch_network_history;
				history->score_network_update = this->branch_score_network->output->acti_vals[0];

				predicted_score += branch_score;

				exit_depth = this->branch_exit_depth;
				exit_node_id = this->branch_next_node_id;

				global_debug_flag = true;
			} else {
				delete branch_network_history;

				history->is_branch = false;
				history->score_network_history = original_network_history;
				history->score_network_update = this->original_score_network->output->acti_vals[0];

				predicted_score += original_score;

				exit_depth = 0;
				exit_node_id = this->original_next_node_id;
			}
		}
	} else {
		StateNetworkHistory* original_network_history = new StateNetworkHistory(this->original_score_network);
		this->original_score_network->activate(local_state_vals,
											   input_vals,
											   original_network_history);

		history->is_branch = false;
		history->score_network_history = original_network_history;
		history->score_network_update = this->original_score_network->output->acti_vals[0];

		predicted_score += scale_factor*this->original_score_network->output->acti_vals[0];

		exit_depth = 0;
		exit_node_id = this->original_next_node_id;
	}
}

void BranchNode::backprop(vector<double>& local_state_errors,
						  vector<double>& input_errors,
						  double target_val,
						  double& predicted_score,
						  double& scale_factor,
						  RunHelper& run_helper,
						  BranchNodeHistory* history) {
	if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN) {
		if (history->is_branch) {
			this->branch_score_network->backprop_errors_with_no_weight_change(
				target_val - predicted_score,
				local_state_errors,
				input_errors,
				history->score_network_history);
		} else {
			this->original_score_network->backprop_errors_with_no_weight_change(
				target_val - predicted_score,
				local_state_errors,
				input_errors,
				history->score_network_history);
		}
	} else if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE) {
		if (history->is_branch) {
			this->branch_score_network->backprop_weights_with_no_error_signal(
				target_val - predicted_score,
				0.002,
				history->score_network_history);
		} else {
			this->original_score_network->backprop_weights_with_no_error_signal(
				target_val - predicted_score,
				0.002,
				history->score_network_history);
		}
	}

	predicted_score -= scale_factor*history->score_network_update;
}

void BranchNode::save(ofstream& output_file,
					  int scope_id,
					  int scope_index) {
	output_file << this->branch_scope_context.size() << endl;
	for (int c_index = 0; c_index < (int)this->branch_scope_context.size(); c_index++) {
		output_file << this->branch_scope_context[c_index] << endl;
		output_file << this->branch_node_context[c_index] << endl;
	}
	output_file << this->branch_is_pass_through << endl;

	ofstream branch_score_network_save_file;
	branch_score_network_save_file.open("saves/nns/" + to_string(scope_id) + "_" + to_string(scope_index) + "_branch_score.txt");
	this->branch_score_network->save(branch_score_network_save_file);
	branch_score_network_save_file.close();

	output_file << this->branch_exit_depth << endl;
	output_file << this->branch_next_node_id << endl;

	ofstream original_score_network_save_file;
	original_score_network_save_file.open("saves/nns/" + to_string(scope_id) + "_" + to_string(scope_index) + "_original_score.txt");
	this->original_score_network->save(original_score_network_save_file);
	original_score_network_save_file.close();

	output_file << this->original_next_node_id << endl;
}

BranchNodeHistory::BranchNodeHistory(BranchNode* node,
									 int scope_index) {
	this->node = node;
	this->scope_index = scope_index;
}

BranchNodeHistory::~BranchNodeHistory() {
	delete this->score_network_history;
}
