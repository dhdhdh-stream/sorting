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
						curr_it->second[context_histories[n_index]->node_id][s_index]->new_outer_activate(
							context_histories[n_index]->obs_snapshot,
							context_histories[n_index]->local_state_vals_snapshot,
							context_histories[n_index]->input_vals_snapshot,
							new_outer_state_vals);
						history->new_outer_state_vals[s_index] += curr_it->second[context_histories[n_index]->node_id][s_index]->output->acti_vals[0];
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
		double outer_state_error = new_outer_state_vals[o_index] - test_new_outer_state_vals[o_index];
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

void Fold::remove_outer_network_update_sequence_activate(
		vector<double>& local_state_vals,
		vector<double>& input_vals,
		vector<vector<double>>& flat_vals,
		double& predicted_score,
		double& scale_factor,
		RunHelper& run_helper,
		FoldHistory* history) {
	vector<double> new_inner_state_vals(this->sum_inner_inputs + this->curr_num_new_inner_states, 0.0);
	vector<double> new_outer_state_vals = history->new_outer_state_vals;

	history->inner_scope_histories = vector<ScopeHistory*>(this->sequence_length, NULL);
	history->score_network_updates = vector<double>(this->sequence_length);
	history->score_network_histories = vector<StateNetworkHistory*>(this->sequence_length, NULL);

	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		if (this->is_inner_scope[f_index]) {
			for (int i_index = 0; i_index < this->inner_input_start_indexes[f_index] + this->num_inner_inputs[f_index]; i_index++) {
				if (!this->curr_state_networks_not_needed[f_index][i_index]) {
					// don't check obs for is_inner_scope
					this->curr_state_networks[f_index][i_index]->new_sequence_activate(
						new_inner_state_vals,
						local_state_vals,
						input_vals,
						new_outer_state_vals);

					new_inner_state_vals[i_index] += this->curr_state_networks[f_index][i_index]->output->acti_vals[0];
				}
			}

			Scope* inner_scope = solution->scopes[this->existing_scope_ids[f_index]];
			vector<double> inner_input_vals(new_inner_state_vals.begin() + this->inner_input_start_indexes[f_index],
				new_inner_state_vals.begin() + this->inner_input_start_indexes[f_index] + this->num_inner_inputs[f_index]);
			int num_input_states_diff = inner_scope->num_input_states - this->num_inner_inputs[f_index];
			inner_input_vals.insert(inner_input_vals.end(), num_input_states_diff, 0.0);
			ScopeHistory* scope_history = new ScopeHistory(inner_scope);
			inner_scope->existing_update_activate(inner_input_vals,
												  flat_vals,
												  predicted_score,
												  scale_factor,
												  run_helper,
												  scope_history);
			history->inner_scope_histories[f_index] = scope_history;
			for (int i_index = 0; i_index < this->num_inner_inputs[f_index]; i_index++) {
				new_inner_state_vals[this->inner_input_start_indexes[f_index] + i_index] = inner_input_vals[i_index];
			}

			// update back state so have chance to compress front after
			for (int i_index = this->inner_input_start_indexes[f_index] + this->num_inner_inputs[f_index];
					i_index < this->sum_inner_inputs + this->curr_num_new_inner_states; i_index++) {
				if (!this->curr_state_networks_not_needed[f_index][i_index]) {
					this->curr_state_networks[f_index][i_index]->new_sequence_activate(
						new_inner_state_vals,
						local_state_vals,
						input_vals,
						new_outer_state_vals);

					new_inner_state_vals[i_index] += this->curr_state_networks[f_index][i_index]->output->acti_vals[0];
				}
			}
			for (int l_index = 0; l_index < this->num_sequence_local_states; l_index++) {
				int state_index = this->sum_inner_inputs
					+ this->curr_num_new_inner_states
					+ l_index;
				if (!this->curr_state_networks_not_needed[f_index][state_index]) {
					this->curr_state_networks[f_index][state_index]->new_sequence_activate(
						new_inner_state_vals,
						local_state_vals,
						input_vals,
						new_outer_state_vals);

					local_state_vals[l_index] += this->curr_state_networks[f_index][state_index]->output->acti_vals[0];
				}
			}
			for (int i_index = 0; i_index < this->num_sequence_input_states; i_index++) {
				int state_index = this->sum_inner_inputs
					+ this->curr_num_new_inner_states
					+ this->num_sequence_local_states
					+ i_index;
				if (!this->curr_state_networks_not_needed[f_index][state_index]) {
					this->curr_state_networks[f_index][state_index]->new_sequence_activate(
						new_inner_state_vals,
						local_state_vals,
						input_vals,
						new_outer_state_vals);

					input_vals[i_index] += this->curr_state_networks[f_index][state_index]->output->acti_vals[0];
				}
			}
			for (int o_index = 0; o_index < this->curr_num_new_outer_states; o_index++) {
				int state_index = this->sum_inner_inputs
					+ this->curr_num_new_inner_states
					+ this->num_sequence_local_states
					+ this->num_sequence_input_states
					+ o_index;
				if (!this->curr_state_networks_not_needed[f_index][state_index]) {
					this->curr_state_networks[f_index][state_index]->new_sequence_activate(
						new_inner_state_vals,
						local_state_vals,
						input_vals,
						new_outer_state_vals);

					new_outer_state_vals[o_index] += this->curr_state_networks[f_index][state_index]->output->acti_vals[0];
				}
			}
		} else {
			for (int i_index = 0; i_index < this->sum_inner_inputs + this->curr_num_new_inner_states; i_index++) {
				if (!this->curr_state_networks_not_needed[f_index][i_index]) {
					this->curr_state_networks[f_index][i_index]->new_sequence_activate(
						*flat_vals.begin(),
						new_inner_state_vals,
						local_state_vals,
						input_vals,
						new_outer_state_vals);

					new_inner_state_vals[i_index] += this->curr_state_networks[f_index][i_index]->output->acti_vals[0];
				}
			}
			for (int l_index = 0; l_index < this->num_sequence_local_states; l_index++) {
				int state_index = this->sum_inner_inputs
					+ this->curr_num_new_inner_states
					+ l_index;
				if (!this->curr_state_networks_not_needed[f_index][state_index]) {
					this->curr_state_networks[f_index][state_index]->new_sequence_activate(
						*flat_vals.begin(),
						new_inner_state_vals,
						local_state_vals,
						input_vals,
						new_outer_state_vals);

					local_state_vals[l_index] += this->curr_state_networks[f_index][state_index]->output->acti_vals[0];
				}
			}
			for (int i_index = 0; i_index < this->num_sequence_input_states; i_index++) {
				int state_index = this->sum_inner_inputs
					+ this->curr_num_new_inner_states
					+ this->num_sequence_local_states
					+ i_index;
				if (!this->curr_state_networks_not_needed[f_index][state_index]) {
					this->curr_state_networks[f_index][state_index]->new_sequence_activate(
						*flat_vals.begin(),
						new_inner_state_vals,
						local_state_vals,
						input_vals,
						new_outer_state_vals);

					input_vals[i_index] += this->curr_state_networks[f_index][state_index]->output->acti_vals[0];
				}
			}
			for (int o_index = 0; o_index < this->curr_num_new_outer_states; o_index++) {
				int state_index = this->sum_inner_inputs
					+ this->curr_num_new_inner_states
					+ this->num_sequence_local_states
					+ this->num_sequence_input_states
					+ o_index;
				if (!this->curr_state_networks_not_needed[f_index][state_index]) {
					this->curr_state_networks[f_index][state_index]->new_sequence_activate(
						*flat_vals.begin(),
						new_inner_state_vals,
						local_state_vals,
						input_vals,
						new_outer_state_vals);

					new_outer_state_vals[o_index] += this->curr_state_networks[f_index][state_index]->output->acti_vals[0];
				}
			}

			flat_vals.erase(flat_vals.begin());
		}

		StateNetworkHistory* score_network_history = new StateNetworkHistory(this->curr_score_networks[f_index]);
		this->curr_score_networks[f_index]->new_sequence_activate(
			new_inner_state_vals,
			local_state_vals,
			input_vals,
			new_outer_state_vals,
			score_network_history);
		history->score_network_updates[f_index] = this->curr_score_networks[f_index]->output->acti_vals[0];
		history->score_network_histories[f_index] = score_network_history;
		predicted_score += scale_factor*this->curr_score_networks[f_index]->output->acti_vals[0];
	}
}

// FOLD_STATE_REMOVE_OUTER_NETWORK update_backprop generic
