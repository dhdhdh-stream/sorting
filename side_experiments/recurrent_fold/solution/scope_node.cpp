#include "scope_node.h"

#include <iostream>

#include "constants.h"
#include "globals.h"

using namespace std;

ScopeNode::ScopeNode(vector<bool> pre_state_network_target_is_local,
					 vector<int> pre_state_network_target_indexes,
					 vector<StateNetwork*> pre_state_networks,
					 int inner_scope_id,
					 vector<bool> inner_input_is_local,
					 vector<int> inner_input_indexes,
					 vector<int> inner_input_target_indexes,
					 vector<bool> post_state_network_target_is_local,
					 vector<int> post_state_network_target_indexes,
					 vector<StateNetwork*> post_state_networks,
					 StateNetwork* score_network) {
	this->type = NODE_TYPE_INNER_SCOPE;

	this->pre_state_network_target_is_local = pre_state_network_target_is_local;
	this->pre_state_network_target_indexes = pre_state_network_target_indexes;
	this->pre_state_networks = pre_state_networks;

	this->inner_scope_id = inner_scope_id;
	this->inner_input_is_local = inner_input_is_local;
	this->inner_input_indexes = inner_input_indexes;
	this->inner_input_target_indexes = inner_input_target_indexes;

	this->post_state_network_target_is_local = post_state_network_target_is_local;
	this->post_state_network_target_indexes = post_state_network_target_indexes;
	this->post_state_networks = post_state_networks;

	this->score_network = score_network;

	// simply initialize to 0.0
	this->average_score = 0.0;
	this->score_variance = 0.0;
	this->average_misguess = 0.0;
	this->misguess_variance = 0.0;
	this->average_impact = 0.0;
	this->average_sum_impact = 0.0;

	this->explore_exit_depth = -1;
	this->explore_next_node_id = -1;
	this->explore_fold = NULL;
	this->explore_loop_fold = NULL;
}

ScopeNode::ScopeNode(ifstream& input_file,
					 int scope_id,
					 int scope_index) {
	this->type = NODE_TYPE_INNER_SCOPE;

	string pre_state_networks_size_line;
	getline(input_file, pre_state_networks_size_line);
	int pre_state_networks_size = stoi(pre_state_networks_size_line);
	for (int s_index = 0; s_index < pre_state_networks_size; s_index++) {
		string target_is_local_line;
		getline(input_file, target_is_local_line);
		this->pre_state_network_target_is_local.push_back(stoi(target_is_local_line));

		string target_index_line;
		getline(input_file, target_index_line);
		this->pre_state_network_target_indexes.push_back(stoi(target_index_line));

		ifstream pre_state_network_save_file;
		pre_state_network_save_file.open("saves/nns/" + to_string(scope_id) + "_" + to_string(scope_index) + "_pre_state_" + to_string(s_index) + ".txt");
		this->pre_state_networks.push_back(new StateNetwork(pre_state_network_save_file));
		pre_state_network_save_file.close();
	}

	string inner_scope_id_line;
	getline(input_file, inner_scope_id_line);
	this->inner_scope_id = stoi(inner_scope_id_line);

	string inner_input_size_line;
	getline(input_file, inner_input_size_line);
	int inner_input_size = stoi(inner_input_size_line);
	for (int i_index = 0; i_index < inner_input_size; i_index++) {
		string input_is_local_line;
		getline(input_file, input_is_local_line);
		this->inner_input_is_local.push_back(stoi(input_is_local_line));

		string input_index_line;
		getline(input_file, input_index_line);
		this->inner_input_indexes.push_back(stoi(input_index_line));

		string input_target_indexes_line;
		getline(input_file, input_target_indexes_line);
		this->inner_input_target_indexes.push_back(stoi(input_target_indexes_line));
	}

	string post_state_networks_size_line;
	getline(input_file, post_state_networks_size_line);
	int post_state_networks_size = stoi(post_state_networks_size_line);
	for (int s_index = 0; s_index < post_state_networks_size; s_index++) {
		string target_is_local_line;
		getline(input_file, target_is_local_line);
		this->post_state_network_target_is_local.push_back(stoi(target_is_local_line));

		string target_index_line;
		getline(input_file, target_index_line);
		this->post_state_network_target_indexes.push_back(stoi(target_index_line));

		ifstream post_state_network_save_file;
		post_state_network_save_file.open("saves/nns/" + to_string(scope_id) + "_" + to_string(scope_index) + "_post_state_" + to_string(s_index) + ".txt");
		this->post_state_networks.push_back(new StateNetwork(post_state_network_save_file));
		post_state_network_save_file.close();
	}

	ifstream score_network_save_file;
	score_network_save_file.open("saves/nns/" + to_string(scope_id) + "_" + to_string(scope_index) + "_score.txt");
	this->score_network = new StateNetwork(score_network_save_file);
	score_network_save_file.close();

	string next_node_id_line;
	getline(input_file, next_node_id_line);
	this->next_node_id = stoi(next_node_id_line);

	string average_score_line;
	getline(input_file, average_score_line);
	this->average_score = stod(average_score_line);

	string score_variance_line;
	getline(input_file, score_variance_line);
	this->score_variance = stod(score_variance_line);

	string average_misguess_line;
	getline(input_file, average_misguess_line);
	this->average_misguess = stod(average_misguess_line);

	string misguess_variance_line;
	getline(input_file, misguess_variance_line);
	this->misguess_variance = stod(misguess_variance_line);

	string average_impact_line;
	getline(input_file, average_impact_line);
	this->average_impact = stod(average_impact_line);

	string average_sum_impact_line;
	getline(input_file, average_sum_impact_line);
	this->average_sum_impact = stod(average_sum_impact_line);

	this->explore_exit_depth = -1;
	this->explore_next_node_id = -1;
	this->explore_fold = NULL;
	this->explore_loop_fold = NULL;
}

ScopeNode::~ScopeNode() {
	for (int s_index = 0; s_index < (int)this->pre_state_networks.size(); s_index++) {
		delete this->pre_state_networks[s_index];
	}

	for (int s_index = 0; s_index < (int)this->post_state_networks.size(); s_index++) {
		delete this->post_state_networks[s_index];
	}

	delete this->score_network;

	if (this->explore_fold != NULL) {
		delete this->explore_fold;
	}

	if (this->explore_loop_fold != NULL) {
		delete this->explore_loop_fold;
	}
}

void ScopeNode::activate(vector<double>& local_state_vals,
						 vector<double>& input_vals,
						 vector<vector<double>>& flat_vals,
						 double& predicted_score,
						 double& scale_factor,
						 double& sum_impact,
						 vector<int>& scope_context,
						 vector<int>& node_context,
						 vector<ScopeHistory*>& context_histories,
						 int& early_exit_depth,
						 int& early_exit_node_id,
						 FoldHistory*& early_exit_fold_history,
						 int& explore_exit_depth,
						 int& explore_exit_node_id,
						 FoldHistory*& explore_exit_fold_history,
						 RunHelper& run_helper,
						 ScopeNodeHistory* history) {
	if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN) {
		for (int s_index = 0; s_index < (int)this->pre_state_networks.size(); s_index++) {
			StateNetworkHistory* network_history = new StateNetworkHistory(this->pre_state_networks[s_index]);
			this->pre_state_networks[s_index]->activate(local_state_vals,
														input_vals,
														network_history);
			history->pre_state_network_histories.push_back(network_history);
			if (this->pre_state_network_target_is_local[s_index]) {
				local_state_vals[this->pre_state_network_target_indexes[s_index]] += this->pre_state_networks[s_index]->output->acti_vals[0];
			} else {
				input_vals[this->pre_state_network_target_indexes[s_index]] += this->pre_state_networks[s_index]->output->acti_vals[0];
			}
		}
	} else {
		for (int s_index = 0; s_index < (int)this->pre_state_networks.size(); s_index++) {
			this->pre_state_networks[s_index]->activate(local_state_vals,
														input_vals);
			if (this->pre_state_network_target_is_local[s_index]) {
				local_state_vals[this->pre_state_network_target_indexes[s_index]] += this->pre_state_networks[s_index]->output->acti_vals[0];
			} else {
				input_vals[this->pre_state_network_target_indexes[s_index]] += this->pre_state_networks[s_index]->output->acti_vals[0];
			}
		}
	}

	Scope* inner_scope = solution->scopes[this->inner_scope_id];
	vector<double> scope_input_vals(inner_scope->num_input_states, 0.0);
	for (int i_index = 0; i_index < (int)this->inner_input_is_local.size(); i_index++) {
		if (this->inner_input_is_local[i_index]) {
			scope_input_vals[this->inner_input_target_indexes[i_index]] = local_state_vals[this->inner_input_indexes[i_index]];
		} else {
			scope_input_vals[this->inner_input_target_indexes[i_index]] = input_vals[this->inner_input_indexes[i_index]];
		}
	}
	ScopeHistory* inner_scope_history = new ScopeHistory(inner_scope);
	inner_scope->activate(scope_input_vals,
						  flat_vals,
						  predicted_score,
						  scale_factor,
						  sum_impact,
						  scope_context,
						  node_context,
						  context_histories,
						  early_exit_depth,
						  early_exit_node_id,
						  early_exit_fold_history,
						  explore_exit_depth,
						  explore_exit_node_id,
						  explore_exit_fold_history,
						  run_helper,
						  inner_scope_history);
	history->inner_scope_history = inner_scope_history;
	// even if early exit, set state_vals
	for (int i_index = 0; i_index < (int)this->inner_input_is_local.size(); i_index++) {
		if (this->inner_input_is_local[i_index]) {
			local_state_vals[this->inner_input_indexes[i_index]] = scope_input_vals[this->inner_input_target_indexes[i_index]];
		} else {
			input_vals[this->inner_input_indexes[i_index]] = scope_input_vals[this->inner_input_target_indexes[i_index]];
		}
	}

	if (early_exit_depth == -1 && explore_exit_depth == -1) {
		history->inner_is_early_exit = false;

		if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN) {
			for (int s_index = 0; s_index < (int)this->post_state_networks.size(); s_index++) {
				StateNetworkHistory* network_history = new StateNetworkHistory(this->post_state_networks[s_index]);
				this->post_state_networks[s_index]->activate(local_state_vals,
															 input_vals,
															 network_history);
				history->post_state_network_histories.push_back(network_history);
				if (this->post_state_network_target_is_local[s_index]) {
					local_state_vals[this->post_state_network_target_indexes[s_index]] += this->post_state_networks[s_index]->output->acti_vals[0];
				} else {
					input_vals[this->post_state_network_target_indexes[s_index]] += this->post_state_networks[s_index]->output->acti_vals[0];
				}
			}
		} else {
			for (int s_index = 0; s_index < (int)this->post_state_networks.size(); s_index++) {
				this->post_state_networks[s_index]->activate(local_state_vals,
															 input_vals);
				if (this->post_state_network_target_is_local[s_index]) {
					local_state_vals[this->post_state_network_target_indexes[s_index]] += this->post_state_networks[s_index]->output->acti_vals[0];
				} else {
					input_vals[this->post_state_network_target_indexes[s_index]] += this->post_state_networks[s_index]->output->acti_vals[0];
				}
			}
		}

		StateNetworkHistory* score_network_history = new StateNetworkHistory(this->score_network);
		this->score_network->activate(local_state_vals,
									  input_vals,
									  score_network_history);
		history->score_network_history = score_network_history;
		history->score_network_update = this->score_network->output->acti_vals[0];
		predicted_score += scale_factor*this->score_network->output->acti_vals[0];
	} else {
		history->inner_is_early_exit = true;
	}
}

void ScopeNode::backprop(vector<double>& local_state_errors,
						 vector<double>& input_errors,
						 double target_val,
						 double final_misguess,
						 double final_sum_impact,
						 double& predicted_score,
						 double& scale_factor,
						 RunHelper& run_helper,
						 ScopeNodeHistory* history) {
	if (!history->inner_is_early_exit) {
		if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN) {
			this->score_network->backprop_errors_with_no_weight_change(
				target_val - predicted_score,
				local_state_errors,
				input_errors,
				history->score_network_history);

			predicted_score -= scale_factor*history->score_network_update;

			for (int s_index = (int)this->post_state_networks.size()-1; s_index >= 0; s_index--) {
				if (this->post_state_network_target_is_local[s_index]) {
					this->post_state_networks[s_index]->backprop_errors_with_no_weight_change(
						local_state_errors[this->post_state_network_target_indexes[s_index]],
						local_state_errors,
						input_errors,
						history->post_state_network_histories[s_index]);
				} else {
					this->post_state_networks[s_index]->backprop_errors_with_no_weight_change(
						input_errors[this->post_state_network_target_indexes[s_index]],
						local_state_errors,
						input_errors,
						history->post_state_network_histories[s_index]);
				}
			}
		} else if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE) {
			this->average_score = 0.9999*this->average_score + 0.0001*target_val;
			double curr_score_variance = (this->average_score - target_val)*(this->average_score - target_val);
			this->score_variance = 0.9999*this->score_variance + 0.0001*curr_score_variance;

			this->average_misguess = 0.9999*this->average_misguess + 0.0001*final_misguess;
			double curr_misguess_variance = (this->average_misguess - final_misguess)*(this->average_misguess - final_misguess);
			this->misguess_variance = 0.9999*this->misguess_variance + 0.0001*curr_misguess_variance;

			this->average_impact = 0.9999*this->average_impact + 0.0001*abs(scale_factor*history->score_network_update);
			this->average_sum_impact = 0.9999*this->average_sum_impact + 0.0001*final_sum_impact;

			this->score_network->backprop_weights_with_no_error_signal(
				target_val - predicted_score,
				0.002,
				history->score_network_history);

			predicted_score -= scale_factor*history->score_network_update;
		}
	}

	Scope* inner_scope = solution->scopes[this->inner_scope_id];
	vector<double> scope_input_errors;
	if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN) {
		scope_input_errors = vector<double>(inner_scope->num_input_states, 0.0);
		for (int i_index = 0; i_index < (int)this->inner_input_is_local.size(); i_index++) {
			if (this->inner_input_is_local[i_index]) {
				scope_input_errors[this->inner_input_target_indexes[i_index]] = local_state_errors[this->inner_input_indexes[i_index]];
			} else {
				scope_input_errors[this->inner_input_target_indexes[i_index]] = input_errors[this->inner_input_indexes[i_index]];
			}
		}
	}
	inner_scope->backprop(scope_input_errors,
						  target_val,
						  final_misguess,
						  final_sum_impact,
						  predicted_score,
						  scale_factor,
						  run_helper,
						  history->inner_scope_history);
	if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN) {
		for (int i_index = 0; i_index < (int)this->inner_input_is_local.size(); i_index++) {
			if (this->inner_input_is_local[i_index]) {
				local_state_errors[this->inner_input_indexes[i_index]] = scope_input_errors[this->inner_input_target_indexes[i_index]];
			} else {
				input_errors[this->inner_input_indexes[i_index]] = scope_input_errors[this->inner_input_target_indexes[i_index]];
			}
		}

		for (int s_index = (int)this->pre_state_networks.size()-1; s_index >= 0; s_index--) {
			if (this->pre_state_network_target_is_local[s_index]) {
				this->pre_state_networks[s_index]->backprop_errors_with_no_weight_change(
					local_state_errors[this->pre_state_network_target_indexes[s_index]],
					local_state_errors,
					input_errors,
					history->pre_state_network_histories[s_index]);
			} else {
				this->pre_state_networks[s_index]->backprop_errors_with_no_weight_change(
					input_errors[this->pre_state_network_target_indexes[s_index]],
					local_state_errors,
					input_errors,
					history->pre_state_network_histories[s_index]);
			}
		}
	}
}

void ScopeNode::save(ofstream& output_file,
					 int scope_id,
					 int scope_index) {
	output_file << this->pre_state_networks.size() << endl;
	for (int s_index = 0; s_index < (int)this->pre_state_networks.size(); s_index++) {
		output_file << this->pre_state_network_target_is_local[s_index] << endl;
		output_file << this->pre_state_network_target_indexes[s_index] << endl;

		ofstream pre_state_network_save_file;
		pre_state_network_save_file.open("saves/nns/" + to_string(scope_id) + "_" + to_string(scope_index) + "_pre_state_" + to_string(s_index) + ".txt");
		this->pre_state_networks[s_index]->save(pre_state_network_save_file);
		pre_state_network_save_file.close();
	}

	output_file << this->inner_scope_id << endl;

	output_file << this->inner_input_is_local.size() << endl;
	for (int i_index = 0; i_index < (int)this->inner_input_is_local.size(); i_index++) {
		output_file << this->inner_input_is_local[i_index] << endl;
		output_file << this->inner_input_indexes[i_index] << endl;
		output_file << this->inner_input_target_indexes[i_index] << endl;
	}

	output_file << this->post_state_networks.size() << endl;
	for (int s_index = 0; s_index < (int)this->post_state_networks.size(); s_index++) {
		output_file << this->post_state_network_target_is_local[s_index] << endl;
		output_file << this->post_state_network_target_indexes[s_index] << endl;

		ofstream post_state_network_save_file;
		post_state_network_save_file.open("saves/nns/" + to_string(scope_id) + "_" + to_string(scope_index) + "_post_state_" + to_string(s_index) + ".txt");
		this->post_state_networks[s_index]->save(post_state_network_save_file);
		post_state_network_save_file.close();
	}

	ofstream score_network_save_file;
	score_network_save_file.open("saves/nns/" + to_string(scope_id) + "_" + to_string(scope_index) + "_score.txt");
	this->score_network->save(score_network_save_file);
	score_network_save_file.close();

	output_file << this->next_node_id << endl;

	output_file << this->average_score << endl;
	output_file << this->score_variance << endl;
	output_file << this->average_misguess << endl;
	output_file << this->misguess_variance << endl;

	output_file << this->average_impact << endl;
	output_file << this->average_sum_impact << endl;
}

ScopeNodeHistory::ScopeNodeHistory(ScopeNode* node,
								   int scope_index) {
	this->node = node;
	this->scope_index = scope_index;

	this->score_network_history = NULL;	// may not trigger
}

ScopeNodeHistory::~ScopeNodeHistory() {
	for (int s_index = 0; s_index < (int)this->pre_state_network_histories.size(); s_index++) {
		delete this->pre_state_network_histories[s_index];
	}

	delete this->inner_scope_history;

	for (int s_index = 0; s_index < (int)this->post_state_network_histories.size(); s_index++) {
		delete this->post_state_network_histories[s_index];
	}

	if (this->score_network_history != NULL) {
		delete this->score_network_history;
	}
}
