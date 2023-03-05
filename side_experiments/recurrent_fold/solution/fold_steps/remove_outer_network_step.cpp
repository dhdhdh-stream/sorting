#include "fold.h"

using namespace std;

void Fold::remove_outer_network_update_score_activate(
		vector<double>& local_state_vals,
		vector<double>& input_vals,
		vector<int>& context_iter,
		vector<ContextHistory*> context_histories,
		RunHelper& run_helper,
		FoldHistory* history) {
	vector<double> new_outer_state_vals(this->curr_num_new_outer_states, 0.0);
	vector<double> test_new_outer_state_vals(this->curr_num_new_outer_states, 0.0);

	int starting_iter = context_iter[context_iter.size() - this->scope_context.size()];
	for (int n_index = starting_iter; n_index < (int)context_histories.size(); n_index++) {
		map<int, vector<vector<StateNetwork*>>>::iterator curr_it = this->curr_outer_state_networks.find(context_histories[n_index]->scope_id);
		if (curr_it != this->curr_outer_state_networks.end()) {
			if (context_histories[n_index]->node_id < curr_it->second.size()) {
				history->outer_state_network_histories.push_back(vector<StateNetworkHistory*>());
				for (int s_index = 0; s_index < this->curr_num_new_outer_states; s_index++) {
					if (!this->curr_outer_state_networks_not_needed[
							context_histories[n_index]->scope_id][context_histories[n_index]->node_id][s_index]) {
						StateNetworkHistory* state_network_history = new StateNetworkHistory(curr_it->second[context_histories[n_index]->node_id][s_index]);
						curr_it->second[context_histories[n_index]->node_id][s_index]->new_outer_activate(
							context_histories[n_index]->obs_snapshot,
							context_histories[n_index]->local_state_vals_snapshot,
							context_histories[n_index]->input_vals_snapshot,
							new_outer_state_vals,
							state_network_history);
						history->outer_state_network_histories.back().push_back(state_network_history);
						history->new_outer_state_vals[s_index] += curr_it->second[context_histories[n_index]->node_id][s_index]->output->acti_vals[0];
					} else {
						history->outer_state_network_histories.back().push_back(NULL);
					}
				}
			}
		}

		map<int, vector<vector<StateNetwork*>>>::iterator test_it = this->test_outer_state_networks.find(context_histories[n_index]->scope_id);
		if (test_it != this->test_outer_state_networks.end()) {
			if (context_histories[n_index]->node_id < test_it->second.size()) {
				history->outer_state_network_histories.push_back(vector<StateNetworkHistory*>());
				for (int s_index = 0; s_index < this->test_num_new_outer_states; s_index++) {
					if (!this->test_outer_state_networks_not_needed[
							context_histories[n_index]->scope_id][context_histories[n_index]->node_id][s_index]) {
						StateNetworkHistory* state_network_history = new StateNetworkHistory(test_it->second[context_histories[n_index]->node_id][s_index]);
						test_it->second[context_histories[n_index]->node_id][s_index]->new_outer_activate(
							context_histories[n_index]->obs_snapshot,
							context_histories[n_index]->local_state_vals_snapshot,
							context_histories[n_index]->input_vals_snapshot,
							test_new_outer_state_vals,
							state_network_history);
						history->test_outer_state_network_histories.back().push_back(state_network_history);
						history->test_new_outer_state_vals[s_index] += test_it->second[context_histories[n_index]->node_id][s_index]->output->acti_vals[0];
					} else {
						history->test_outer_state_network_histories.back().push_back(NULL);
					}
				}
			}
		}
	}

	vector<double> test_new_outer_state_errors(this->curr_num_new_outer_states, 0.0);
	for (int o_index = 0; o_index < this->curr_num_new_outer_states; o_index++) {
		double outer_state_error = this->new_outer_state_vals[o_index] - this->test_new_outer_state_vals[o_index];
		this->sum_error += abs(outer_state_error);
		test_new_outer_state_errors[o_index] = outer_state_error;
	}

	double target_max_update;
	if (this->state_iter <= 130000) {
		target_max_update = 0.01;
	} else {
		target_max_update = 0.002;
	}
	for (int n_index = history->test_outer_state_network_histories.size()-1; n_index >= 0; n_index--) {
		for (int o_index = this->curr_num_new_outer_states-1; o_index >= 0; o_index--) {
			if (history->test_outer_state_network_histories[n_index][o_index] != NULL) {
				StateNetwork* state_network = history->test_outer_state_network_histories[n_index][o_index]->network;
				state_network->new_outer_backprop(
					test_new_outer_state_errors[o_index],
					test_new_outer_state_errors,
					target_max_update,
					history->test_outer_state_network_histories[n_index][o_index]);
			}
		}
	}

	StateNetworkHistory* starting_score_network_history = new StateNetworkHistory(this->curr_starting_score_network);
	this->curr_starting_score_network->new_outer_activate(
		local_state_vals,
		input_vals,
		new_outer_state_vals,
		starting_score_network_history);
	history->starting_score_update = this->curr_starting_score_network->output->acti_vals[0];
	history->starting_score_network_history = starting_score_network_history;

	history->new_outer_state_vals = new_outer_state_vals;

	// modify predicted_score if path taken outside
}

// FOLD_STATE_REMOVE_OUTER_NETWORK update_sequence_activate generic

// FOLD_STATE_REMOVE_OUTER_NETWORK update_backprop generic
