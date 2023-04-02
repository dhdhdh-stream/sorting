#include "action_node.h"

#include <iostream>

#include "constants.h"

using namespace std;

ActionNode::ActionNode(vector<bool> state_network_target_is_local,
					   vector<int> state_network_target_indexes,
					   vector<StateNetwork*> state_networks,
					   StateNetwork* score_network) {
	this->type = NODE_TYPE_ACTION;

	this->state_network_target_is_local = state_network_target_is_local;
	this->state_network_target_indexes = state_network_target_indexes;
	this->state_networks = state_networks;
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
}

ActionNode::ActionNode(ifstream& input_file,
					   int scope_id,
					   int scope_index) {
	this->type = NODE_TYPE_ACTION;

	string state_networks_size_line;
	getline(input_file, state_networks_size_line);
	int state_networks_size = stoi(state_networks_size_line);
	for (int s_index = 0; s_index < state_networks_size; s_index++) {
		string target_is_local_line;
		getline(input_file, target_is_local_line);
		this->state_network_target_is_local.push_back(stoi(target_is_local_line));

		string target_index_line;
		getline(input_file, target_index_line);
		this->state_network_target_indexes.push_back(stoi(target_index_line));

		ifstream state_network_save_file;
		state_network_save_file.open("saves/nns/" + to_string(scope_id) + "_" + to_string(scope_index) + "_state_" + to_string(s_index) + ".txt");
		this->state_networks.push_back(new StateNetwork(state_network_save_file));
		state_network_save_file.close();
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
}

ActionNode::~ActionNode() {
	for (int s_index = 0; s_index < (int)this->state_networks.size(); s_index++) {
		delete this->state_networks[s_index];
	}

	delete this->score_network;
}

void ActionNode::activate(vector<double>& local_state_vals,
						  vector<double>& input_vals,
						  vector<vector<double>>& flat_vals,
						  double& predicted_score,
						  double& scale_factor,
						  RunHelper& run_helper,
						  ActionNodeHistory* history) {
	double obs = (*flat_vals.begin())[0];

	history->obs_snapshot = obs;

	if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN) {
		for (int s_index = 0; s_index < (int)this->state_networks.size(); s_index++) {
			StateNetworkHistory* network_history = new StateNetworkHistory(this->state_networks[s_index]);
			this->state_networks[s_index]->activate(obs,
													local_state_vals,
													input_vals,
													network_history);
			history->state_network_histories.push_back(network_history);
			if (this->state_network_target_is_local[s_index]) {
				local_state_vals[this->state_network_target_indexes[s_index]] += this->state_networks[s_index]->output->acti_vals[0];
			} else {
				input_vals[this->state_network_target_indexes[s_index]] += this->state_networks[s_index]->output->acti_vals[0];
			}
		}
	} else {
		for (int s_index = 0; s_index < (int)this->state_networks.size(); s_index++) {
			this->state_networks[s_index]->activate(obs,
													local_state_vals,
													input_vals);
			if (this->state_network_target_is_local[s_index]) {
				local_state_vals[this->state_network_target_indexes[s_index]] += this->state_networks[s_index]->output->acti_vals[0];
			} else {
				input_vals[this->state_network_target_indexes[s_index]] += this->state_networks[s_index]->output->acti_vals[0];
			}
		}
	}

	flat_vals.erase(flat_vals.begin());

	StateNetworkHistory* network_history = new StateNetworkHistory(this->score_network);
	this->score_network->activate(local_state_vals,
								  input_vals,
								  network_history);
	history->score_network_history = network_history;
	history->score_network_update = this->score_network->output->acti_vals[0];
	predicted_score += scale_factor*this->score_network->output->acti_vals[0];

	history->ending_local_state_snapshot = local_state_vals;
	history->ending_input_state_snapshot = input_vals;
}

void ActionNode::backprop(vector<double>& local_state_errors,
						  vector<double>& input_errors,
						  double target_val,
						  double final_misguess,
						  double final_sum_impact,
						  double& predicted_score,
						  double& scale_factor,
						  RunHelper& run_helper,
						  ActionNodeHistory* history) {
	if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN) {
		this->score_network->backprop_errors_with_no_weight_change(
			target_val - predicted_score,
			local_state_errors,
			input_errors,
			history->score_network_history);

		predicted_score -= scale_factor*history->score_network_update;

		for (int s_index = (int)this->state_networks.size()-1; s_index >= 0; s_index--) {
			if (this->state_network_target_is_local[s_index]) {
				this->state_networks[s_index]->backprop_errors_with_no_weight_change(
					local_state_errors[this->state_network_target_indexes[s_index]],
					local_state_errors,
					input_errors,
					history->state_network_histories[s_index]);
			} else {
				this->state_networks[s_index]->backprop_errors_with_no_weight_change(
					input_errors[this->state_network_target_indexes[s_index]],
					local_state_errors,
					input_errors,
					history->state_network_histories[s_index]);
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

void ActionNode::save(ofstream& output_file,
					  int scope_id,
					  int scope_index) {
	output_file << this->state_networks.size() << endl;
	for (int s_index = 0; s_index < (int)this->state_networks.size(); s_index++) {
		output_file << this->state_network_target_is_local[s_index] << endl;
		output_file << this->state_network_target_indexes[s_index] << endl;

		ofstream state_network_save_file;
		state_network_save_file.open("saves/nns/" + to_string(scope_id) + "_" + to_string(scope_index) + "_state_" + to_string(s_index) + ".txt");
		this->state_networks[s_index]->save(state_network_save_file);
		state_network_save_file.close();
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

ActionNodeHistory::ActionNodeHistory(ActionNode* node,
									 int scope_index) {
	this->node = node;
	this->scope_index = scope_index;
}

ActionNodeHistory::~ActionNodeHistory() {
	for (int s_index = 0; s_index < (int)this->state_network_histories.size(); s_index++) {
		delete this->state_network_histories[s_index];
	}

	delete this->score_network_history;
}
