#include "action_node.h"

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
}

ActionNode::~ActionNode() {
	for (int s_index = 0; s_index < (int)this->state_networks.size(); s_index++) {
		delete this->state_networks[s_index];
	}

	delete this->score_network;
}

void ActionNode::explore_on_path_activate(vector<double>& local_state_vals,
										  vector<double>& input_vals,
										  vector<vector<double>>& flat_vals,
										  double& predicted_score,
										  double& scale_factor,
										  RunHelper& run_helper,
										  ActionNodeHistory* history) {
	vector<double> obs = *flat_vals.begin();

	if (run_helper.explore_phase == EXPLORE_PHASE_FLAT) {
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

	if (run_helper.explore_phase == EXPLORE_PHASE_FLAT) {
		StateNetworkHistory* network_history = new StateNetworkHistory(this->score_network);
		this->score_network->activate(local_state_vals,
									  input_vals,
									  network_history);
		history->score_network_history = network_history;
		history->score_network_update = this->score_network->output->acti_vals[0];
	} else {
		this->score_network->activate(local_state_vals,
									  input_vals);
	}
	predicted_score += scale_factor*this->score_network->output->acti_vals[0];
}

void ActionNode::explore_on_path_backprop(vector<double>& local_state_errors,
										  vector<double>& input_errors,
										  double target_val,
										  double& predicted_score,
										  double& scale_factor,
										  ActionNodeHistory* history) {
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
}

void ActionNode::update_activate(vector<double>& local_state_vals,
								 vector<double>& input_vals,
								 vector<vector<double>>& flat_vals,
								 double& predicted_score,
								 double& scale_factor,
								 double& sum_impact,
								 double& explore_weight_scale_factor,
								 ActionNodeHistory* history) {
	vector<double> obs = *flat_vals.begin();

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

	flat_vals.erase(flat_vals.begin());

	StateNetworkHistory* network_history = new StateNetworkHistory(this->score_network);
	this->score_network->activate(local_state_vals,
								  input_vals,
								  network_history);
	history->score_network_history = network_history;
	history->score_network_update = this->score_network->output->acti_vals[0];

	predicted_score += scale_factor*this->score_network->output->acti_vals[0];

	sum_impact += explore_weight_scale_factor*this->average_impact;
	// use average_impact instead of instance impact to smooth calculation(?)
}

void ActionNode::update_backprop(double target_val,
								 double final_misguess,
								 double& predicted_score,
								 double& scale_factor,
								 double sum_impact_error,
								 double& explore_weight_scale_factor_error,
								 ActionNodeHistory* history) {
	this->average_score = 0.9999*this->average_score + 0.0001*target_val;
	double curr_score_variance = (this->average_score - target_val)*(this->average_score - target_val);
	this->score_variance = 0.9999*this->score_variance + 0.0001*curr_score_variance;

	this->average_misguess = 0.9999*this->average_misguess + 0.0001*final_misguess;
	double curr_misguess_variance = (this->average_misguess - final_misguess)*(this->average_misguess - final_misguess);
	this->misguess_variance = 0.9999*this->misguess_variance + 0.0001*curr_misguess_variance;

	this->average_impact = 0.9999*this->average_impact + 0.0001*abs(history->score_network_update);

	explore_weight_scale_factor_error += sum_impact_error*this->average_impact;

	this->score_network->backprop_weights_with_no_error_signal(
		target_val - predicted_score,
		0.002,
		history->score_network_history);

	predicted_score -= scale_factor*history->score_network_update;
}

ActionNodeHistory::ActionNodeHistory(ActionNode* node) {
	this->node = node;
}

ActionNodeHistory::~ActionNodeHistory() {
	for (int s_index = 0; s_index < (int)this->state_network_histories.size(); s_index++) {
		delete this->state_network_histories[s_index];
	}

	delete this->score_network_history;
}
