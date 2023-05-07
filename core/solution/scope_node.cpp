#include "scope_node.h"

#include <iostream>

#include "constants.h"
#include "globals.h"

using namespace std;

ScopeNode::ScopeNode(vector<int> pre_state_network_target_indexes,
					 vector<StateNetwork*> pre_state_networks,
					 int inner_scope_id,
					 vector<int> inner_input_indexes,
					 vector<int> inner_input_target_indexes,
					 Scale* scope_scale_mod,
					 vector<int> post_state_network_target_indexes,
					 vector<StateNetwork*> post_state_networks,
					 StateNetwork* score_network) {
	this->type = NODE_TYPE_INNER_SCOPE;

	this->pre_state_network_target_indexes = pre_state_network_target_indexes;
	this->pre_state_networks = pre_state_networks;

	this->inner_scope_id = inner_scope_id;
	this->inner_input_indexes = inner_input_indexes;
	this->inner_input_target_indexes = inner_input_target_indexes;
	this->scope_scale_mod = scope_scale_mod;

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

	this->explore_curr_try = 0;
	this->explore_target_tries = 1;
	int rand_scale = rand()%4;
	for (int i = 0; i < rand_scale; i++) {
		this->explore_target_tries *= 10;
	}
	this->best_explore_surprise = numeric_limits<double>::lowest();
	this->best_explore_seed_outer_context_history = NULL;

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
		string input_index_line;
		getline(input_file, input_index_line);
		this->inner_input_indexes.push_back(stoi(input_index_line));

		string input_target_indexes_line;
		getline(input_file, input_target_indexes_line);
		this->inner_input_target_indexes.push_back(stoi(input_target_indexes_line));
	}

	string scope_scale_mod_weight_line;
	getline(input_file, scope_scale_mod_weight_line);
	this->scope_scale_mod = new Scale(stof(scope_scale_mod_weight_line));

	string post_state_networks_size_line;
	getline(input_file, post_state_networks_size_line);
	int post_state_networks_size = stoi(post_state_networks_size_line);
	for (int s_index = 0; s_index < post_state_networks_size; s_index++) {
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

	this->explore_curr_try = 0;
	this->explore_target_tries = 1;
	int rand_scale = rand()%4;
	for (int i = 0; i < rand_scale; i++) {
		this->explore_target_tries *= 10;
	}
	this->best_explore_surprise = numeric_limits<double>::lowest();
	this->best_explore_seed_outer_context_history = NULL;

	this->explore_exit_depth = -1;
	this->explore_next_node_id = -1;
	this->explore_fold = NULL;
	this->explore_loop_fold = NULL;
}

ScopeNode::~ScopeNode() {
	for (int s_index = 0; s_index < (int)this->pre_state_networks.size(); s_index++) {
		delete this->pre_state_networks[s_index];
	}

	delete this->scope_scale_mod;

	for (int s_index = 0; s_index < (int)this->post_state_networks.size(); s_index++) {
		delete this->post_state_networks[s_index];
	}

	delete this->score_network;

	if (this->best_explore_seed_outer_context_history != NULL) {
		delete this->best_explore_seed_outer_context_history;
	}

	if (this->explore_fold != NULL) {
		delete this->explore_fold;
	}

	if (this->explore_loop_fold != NULL) {
		delete this->explore_loop_fold;
	}
}

void ScopeNode::activate(Problem& problem,
						 vector<double>& state_vals,
						 vector<bool>& states_initialized,
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
		history->pre_state_network_histories = vector<StateNetworkHistory*>(this->pre_state_networks.size(), NULL);
		for (int s_index = 0; s_index < (int)this->pre_state_networks.size(); s_index++) {
			if (states_initialized[this->pre_state_network_target_indexes[s_index]]) {
				StateNetworkHistory* network_history = new StateNetworkHistory(this->pre_state_networks[s_index]);
				this->pre_state_networks[s_index]->activate(state_vals,
															network_history);
				history->pre_state_network_histories[s_index] = network_history;
				state_vals[this->pre_state_network_target_indexes[s_index]] += this->pre_state_networks[s_index]->output->acti_vals[0];
			}
		}
	} else {
		for (int s_index = 0; s_index < (int)this->pre_state_networks.size(); s_index++) {
			if (states_initialized[this->pre_state_network_target_indexes[s_index]]) {
				this->pre_state_networks[s_index]->activate(state_vals);
				state_vals[this->pre_state_network_target_indexes[s_index]] += this->pre_state_networks[s_index]->output->acti_vals[0];
			}
		}
	}

	scale_factor *= this->scope_scale_mod->weight;

	Scope* inner_scope = solution->scopes[this->inner_scope_id];
	vector<double> scope_input_vals(inner_scope->num_states, 0.0);
	vector<bool> scope_inputs_initialized(inner_scope->num_states, false);
	for (int i_index = 0; i_index < (int)this->inner_input_indexes.size(); i_index++) {
		if (states_initialized[this->inner_input_indexes[i_index]]) {
			scope_input_vals[this->inner_input_target_indexes[i_index]] = state_vals[this->inner_input_indexes[i_index]];
			scope_inputs_initialized[this->inner_input_target_indexes[i_index]] = true;
		}
	}
	ScopeHistory* inner_scope_history = new ScopeHistory(inner_scope);
	inner_scope->activate(problem,
						  scope_input_vals,
						  scope_inputs_initialized,
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
	for (int i_index = 0; i_index < (int)this->inner_input_indexes.size(); i_index++) {
		if (states_initialized[this->inner_input_indexes[i_index]]) {
			state_vals[this->inner_input_indexes[i_index]] = scope_input_vals[this->inner_input_target_indexes[i_index]];
		}
	}

	scale_factor /= this->scope_scale_mod->weight;

	if (early_exit_depth == -1 && explore_exit_depth == -1) {
		history->inner_is_early_exit = false;

		if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN) {
			history->post_state_network_histories = vector<StateNetworkHistory*>(this->post_state_networks.size(), NULL);
			for (int s_index = 0; s_index < (int)this->post_state_networks.size(); s_index++) {
				if (states_initialized[this->post_state_network_target_indexes[s_index]]) {
					StateNetworkHistory* network_history = new StateNetworkHistory(this->post_state_networks[s_index]);
					this->post_state_networks[s_index]->activate(state_vals,
																 network_history);
					history->post_state_network_histories[s_index] = network_history;
					state_vals[this->post_state_network_target_indexes[s_index]] += this->post_state_networks[s_index]->output->acti_vals[0];
				}
			}
		} else {
			for (int s_index = 0; s_index < (int)this->post_state_networks.size(); s_index++) {
				if (states_initialized[this->post_state_network_target_indexes[s_index]]) {
					this->post_state_networks[s_index]->activate(state_vals);
					state_vals[this->post_state_network_target_indexes[s_index]] += this->post_state_networks[s_index]->output->acti_vals[0];
				}
			}
		}

		StateNetworkHistory* score_network_history = new StateNetworkHistory(this->score_network);
		this->score_network->activate(state_vals,
									  score_network_history);
		history->score_network_history = score_network_history;
		history->score_network_update = this->score_network->output->acti_vals[0];
		predicted_score += scale_factor*this->score_network->output->acti_vals[0];
	} else {
		history->inner_is_early_exit = true;
	}
}

void ScopeNode::backprop(vector<double>& state_errors,
						 vector<bool>& states_initialized,
						 double target_val,
						 double final_diff,
						 double final_misguess,
						 double final_sum_impact,
						 double& predicted_score,
						 double& scale_factor,
						 double& scale_factor_error,
						 RunHelper& run_helper,
						 ScopeNodeHistory* history) {
	if (!history->inner_is_early_exit) {
		if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN) {
			double predicted_score_error = target_val - predicted_score;
			this->score_network->backprop_errors_with_no_weight_change(
				scale_factor*predicted_score_error,
				state_errors,
				history->score_network_history);

			predicted_score -= scale_factor*history->score_network_update;

			for (int s_index = (int)this->post_state_networks.size()-1; s_index >= 0; s_index--) {
				if (states_initialized[this->post_state_network_target_indexes[s_index]]) {
					this->post_state_networks[s_index]->backprop_errors_with_no_weight_change(
						state_errors[this->post_state_network_target_indexes[s_index]],
						state_errors,
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

			double predicted_score_error = target_val - predicted_score;

			scale_factor_error += history->score_network_update*predicted_score_error;

			this->score_network->backprop_weights_with_no_error_signal(
				scale_factor*predicted_score_error,
				0.002,
				history->score_network_history);

			predicted_score -= scale_factor*history->score_network_update;
		}
	}

	scale_factor *= this->scope_scale_mod->weight;

	Scope* inner_scope = solution->scopes[this->inner_scope_id];
	vector<double> scope_input_errors;
	vector<bool> scope_inputs_initialized;
	if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN) {
		scope_input_errors = vector<double>(inner_scope->num_states, 0.0);
		scope_inputs_initialized = vector<bool>(inner_scope->num_states, false);
		for (int i_index = 0; i_index < (int)this->inner_input_indexes.size(); i_index++) {
			if (states_initialized[this->inner_input_indexes[i_index]]) {
				scope_input_errors[this->inner_input_target_indexes[i_index]] = state_errors[this->inner_input_indexes[i_index]];
				scope_inputs_initialized[this->inner_input_target_indexes[i_index]] = true;
			}
		}
	}
	double scope_scale_factor_error = 0.0;
	inner_scope->backprop(scope_input_errors,
						  scope_inputs_initialized,
						  target_val,
						  final_diff,
						  final_misguess,
						  final_sum_impact,
						  predicted_score,
						  scale_factor,
						  scope_scale_factor_error,
						  run_helper,
						  history->inner_scope_history);

	scale_factor /= this->scope_scale_mod->weight;

	if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN) {
		for (int i_index = 0; i_index < (int)this->inner_input_indexes.size(); i_index++) {
			if (states_initialized[this->inner_input_indexes[i_index]]) {
				state_errors[this->inner_input_indexes[i_index]] = scope_input_errors[this->inner_input_target_indexes[i_index]];
			}
		}

		for (int s_index = (int)this->pre_state_networks.size()-1; s_index >= 0; s_index--) {
			if (states_initialized[this->pre_state_network_target_indexes[s_index]]) {
				this->pre_state_networks[s_index]->backprop_errors_with_no_weight_change(
					state_errors[this->pre_state_network_target_indexes[s_index]],
					state_errors,
					history->pre_state_network_histories[s_index]);
			}
		}
	} else if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE) {
		this->scope_scale_mod->backprop(scope_scale_factor_error, 0.0002);

		scale_factor_error += this->scope_scale_mod->weight*scope_scale_factor_error;
	}
}

void ScopeNode::save(ofstream& output_file,
					 int scope_id,
					 int scope_index) {
	output_file << this->pre_state_networks.size() << endl;
	for (int s_index = 0; s_index < (int)this->pre_state_networks.size(); s_index++) {
		output_file << this->pre_state_network_target_indexes[s_index] << endl;

		ofstream pre_state_network_save_file;
		pre_state_network_save_file.open("saves/nns/" + to_string(scope_id) + "_" + to_string(scope_index) + "_pre_state_" + to_string(s_index) + ".txt");
		this->pre_state_networks[s_index]->save(pre_state_network_save_file);
		pre_state_network_save_file.close();
	}

	output_file << this->inner_scope_id << endl;

	output_file << this->inner_input_indexes.size() << endl;
	for (int i_index = 0; i_index < (int)this->inner_input_indexes.size(); i_index++) {
		output_file << this->inner_input_indexes[i_index] << endl;
		output_file << this->inner_input_target_indexes[i_index] << endl;
	}

	output_file << this->scope_scale_mod->weight << endl;

	output_file << this->post_state_networks.size() << endl;
	for (int s_index = 0; s_index < (int)this->post_state_networks.size(); s_index++) {
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

void ScopeNode::save_for_display(ofstream& output_file) {
	output_file << this->inner_scope_id << endl;

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

ScopeNodeHistory::ScopeNodeHistory(ScopeNodeHistory* original) {
	this->node = original->node;
	this->scope_index = original->scope_index;

	this->score_network_history = NULL;

	this->inner_scope_history = new ScopeHistory(original->inner_scope_history);
}

ScopeNodeHistory::~ScopeNodeHistory() {
	for (int s_index = 0; s_index < (int)this->pre_state_network_histories.size(); s_index++) {
		if (this->pre_state_network_histories[s_index] != NULL) {
			delete this->pre_state_network_histories[s_index];
		}
	}

	delete this->inner_scope_history;

	for (int s_index = 0; s_index < (int)this->post_state_network_histories.size(); s_index++) {
		if (this->post_state_network_histories[s_index] != NULL) {
			delete this->post_state_network_histories[s_index];
		}
	}

	if (this->score_network_history != NULL) {
		delete this->score_network_history;
	}
}
