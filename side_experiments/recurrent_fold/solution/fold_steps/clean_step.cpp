#include "fold.h"

using namespace std;

void Fold::clean_update_sequence_activate(vector<double>& local_state_vals,
										  vector<double>& input_vals,
										  vector<vector<double>>& flat_vals,
										  double& predicted_score,
										  double& scale_factor,
										  RunHelper& run_helper,
										  FoldHistory* history) {
	vector<double> new_inner_state_vals(this->sum_inner_inputs + this->curr_num_new_inner_states, 0.0);
	vector<double> new_outer_state_vals = history->new_outer_state_vals;

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

void Fold::clean_update_backprop(double target_val,
								 double final_misguess,
								 double& predicted_score,
								 double& scale_factor,
								 FoldHistory* history) {
	for (int f_index = this->sequence_length-1; f_index >= 0; f_index--) {
		this->curr_score_networks[f_index]->backprop_weights_with_no_error_signal(
			target_val - predicted_score,
			0.002,
			history->score_network_histories[f_index]);

		predicted_score -= scale_factor*history->score_network_updates[f_index];

		if (this->is_inner_scope[f_index]) {
			Scope* inner_scope = solution->scopes[this->existing_scope_ids[f_index]];
			// TODO: calc scale_factor_error
			inner_scope->existing_update_backprop(predicted_score,
												  scale_factor,
												  history->inner_scope_histories[f_index]);
		}
	}

	this->curr_starting_score_network->backprop_weights_with_no_error_signal(
		target_val - predicted_score,
		0.002,
		history->starting_score_network_history);

	predicted_score -= scale_factor*history->starting_score_update;
}
