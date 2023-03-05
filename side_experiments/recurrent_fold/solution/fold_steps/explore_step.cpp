/**
 * Used for FOLD_STATE_EXPLORE, FOLD_STATE_ADD_INNER_STATE, and FOLD_STATE_ADD_OUTER_STATE
 */

#include "fold.h"

using namespace std;

void Fold::explore_on_path_score_activate(
		vector<double>& local_state_vals,
		vector<double>& input_vals,
		double& predicted_score,
		double& scale_factor,
		vector<int>& context_iter,
		vector<ContextHistory*> context_histories,
		RunHelper& run_helper,
		FoldHistory* history) {
	vector<double> new_outer_state_vals(this->test_num_new_outer_states, 0.0);

	int starting_iter = context_iter[context_iter.size() - this->scope_context.size()];
	for (int n_index = starting_iter; n_index < (int)context_histories.size(); n_index++) {
		map<int, vector<vector<Network*>>>::iterator it = this->test_outer_state_networks.find(context_histories[n_index]->scope_id);
		if (it == this->test_outer_state_networks.end()) {
			it = this->test_outer_state_networks.insert({context_histories[n_index]->scope_id, vector<vector<Network*>>()}).first;
		}

		if (context_histories[n_index]->node_id >= it->second.size()) {
			int size_diff = solution->scopes[context_histories[n_index]->scope_id]->nodes.size() - it->second.size();
			it->second.insert(it->second.begin(), size_diff, vector<Network*>());
		}

		if (it->second[context_histories[n_index]->node_id].size() == 0) {
			for (int s_index = 0; s_index < this->test_num_new_outer_states; s_index++) {
				it->second[context_histories[n_index]->node_id].push_back(new StateNetwork(
					1,
					solution->scopes[context_histories[n_index]->scope_id]->num_local_states,
					solution->scopes[context_histories[n_index]->scope_id]->num_input_states,
					0,
					this->test_num_new_outer_states,
					20));
			}
		}

		history->outer_state_network_histories.push_back(vector<StateNetworkHistory*>());
		for (int s_index = 0; s_index < this->test_num_new_outer_states; s_index++) {
			StateNetworkHistory* state_network_history = new StateNetworkHistory(it->second[context_histories[n_index]->node_id][s_index]);
			it->second[context_histories[n_index]->node_id][s_index]->new_outer_activate(
				context_histories[n_index]->obs_snapshot,
				context_histories[n_index]->local_state_vals_snapshot,
				context_histories[n_index]->input_vals_snapshot,
				new_outer_state_vals,
				state_network_history);
			history->outer_state_network_histories.back().push_back(state_network_history);
			history->new_outer_state_vals[s_index] += it->second[context_histories[n_index]->node_id][s_index]->output->acti_vals[0];
		}
	}

	StateNetworkHistory* starting_score_network_history = new StateNetworkHistory(this->test_starting_score_network);
	this->test_starting_score_network->new_outer_activate(
		local_state_vals,
		input_vals,
		new_outer_state_vals,
		starting_score_network_history);
	history->starting_score_update = this->test_starting_score_network->output->acti_vals[0];
	history->starting_score_network_history = starting_score_network_history;

	history->new_outer_state_vals = new_outer_state_vals;

	if (this->state == FOLD_STATE_EXPLORE && this->state_iter >= 490000) {
		// Note: use predicted_score_variance (as opposed to score_variance for surprise) to measure difference between predicted scores
		double predicted_score_standard_deviation = sqrt(this->existing_predicted_score_variance);
		double t_value = scale_factor*this->test_starting_score_network->output->acti_vals[0] / predicted_score_standard_deviation;
		if (t_value > 1.0) {	// >75%
			this->new_noticably_better++;
		} else if (t_value < -1.0) {	// >75%
			this->existing_noticably_better++;
		}
	}

	// modify predicted_score in score_activate for explore
	predicted_score += scale_factor*this->test_starting_score_network->output->acti_vals[0];
}

void Fold::explore_on_path_sequence_activate(vector<double>& local_state_vals,
											 vector<double>& input_vals,
											 vector<vector<double>>& flat_vals,
											 double& predicted_score,
											 double& scale_factor,
											 RunHelper& run_helper,
											 FoldHistory* history) {
	vector<double> new_inner_state_vals(this->sum_inner_inputs + this->test_num_new_inner_states, 0.0);
	vector<double> new_outer_state_vals = history->new_outer_state_vals;
	
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		if (this->is_inner_scope[f_index]) {
			for (int i_index = 0; i_index < this->inner_input_start_indexes[f_index] + this->num_inner_inputs[f_index]; i_index++) {
				StateNetworkHistory* state_network_history = new StateNetworkHistory(this->test_state_networks[f_index][i_index]);
				// don't check obs for is_inner_scope
				this->test_state_networks[f_index][i_index]->new_sequence_activate(
					new_inner_state_vals,
					local_state_vals,
					input_vals,
					new_outer_state_vals,
					state_network_history);
				history->state_network_histories[f_index][i_index] = state_network_history;

				new_inner_state_vals[i_index] += this->test_state_networks[f_index][i_index]->output->acti_vals[0];
			}

			Scope* inner_scope = solution->scopes[this->existing_scope_ids[f_index]];
			vector<double> inner_input_vals(new_inner_state_vals.begin() + this->inner_input_start_indexes[f_index],
				new_inner_state_vals.begin() + this->inner_input_start_indexes[f_index] + this->num_inner_inputs[f_index]);
			int num_input_states_diff = inner_scope->num_input_states - this->num_inner_inputs[f_index];
			inner_input_vals.insert(inner_input_vals.end(), num_input_states_diff, 0.0);
			ScopeHistory* scope_history = new ScopeHistory(inner_scope);
			inner_scope->existing_explore_activate(inner_input_vals,
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
					i_index < this->sum_inner_inputs + this->test_num_new_inner_states; i_index++) {
				StateNetworkHistory* state_network_history = new StateNetworkHistory(this->test_state_networks[f_index][i_index]);
				this->test_state_networks[f_index][i_index]->new_sequence_activate(
					new_inner_state_vals,
					local_state_vals,
					input_vals,
					new_outer_state_vals,
					state_network_history);
				history->state_network_histories[f_index][i_index] = state_network_history;

				new_inner_state_vals[i_index] += this->test_state_networks[f_index][i_index]->output->acti_vals[0];
			}
			for (int l_index = 0; l_index < this->num_sequence_local_states; l_index++) {
				int state_index = this->sum_inner_inputs
					+ this->test_num_new_inner_states
					+ l_index;
				StateNetworkHistory* state_network_history = new StateNetworkHistory(this->test_state_networks[f_index][state_index]);
				this->test_state_networks[f_index][state_index]->new_sequence_activate(
					new_inner_state_vals,
					local_state_vals,
					input_vals,
					new_outer_state_vals,
					state_network_history);
				history->state_network_histories[f_index][state_index] = state_network_history;

				local_state_vals[l_index] += this->test_state_networks[f_index][state_index]->output->acti_vals[0];
			}
			for (int i_index = 0; i_index < this->num_sequence_input_states; i_index++) {
				int state_index = this->sum_inner_inputs
					+ this->test_num_new_inner_states
					+ this->num_sequence_local_states
					+ i_index;
				StateNetworkHistory* state_network_history = new StateNetworkHistory(this->test_state_networks[f_index][state_index]);
				this->test_state_networks[f_index][state_index]->new_sequence_activate(
					new_inner_state_vals,
					local_state_vals,
					input_vals,
					new_outer_state_vals,
					state_network_history);
				history->state_network_histories[f_index][state_index] = state_network_history;

				input_vals[i_index] += this->test_state_networks[f_index][state_index]->output->acti_vals[0];
			}
			for (int o_index = 0; o_index < this->test_num_new_outer_states; o_index++) {
				int state_index = this->sum_inner_inputs
					+ this->test_num_new_inner_states
					+ this->num_sequence_local_states
					+ this->num_sequence_input_states
					+ o_index;
				StateNetworkHistory* state_network_history = new StateNetworkHistory(this->test_state_networks[f_index][state_index]);
				this->test_state_networks[f_index][state_index]->new_sequence_activate(
					new_inner_state_vals,
					local_state_vals,
					input_vals,
					new_outer_state_vals,
					state_network_history);
				history->state_network_histories[f_index][state_index] = state_network_history;

				new_outer_state_vals[o_index] += this->test_state_networks[f_index][state_index]->output->acti_vals[0];
			}
		} else {
			for (int i_index = 0; i_index < this->sum_inner_inputs + this->test_num_new_inner_states; i_index++) {
				StateNetworkHistory* state_network_history = new StateNetworkHistory(this->test_state_networks[f_index][i_index]);
				this->test_state_networks[f_index][i_index]->new_sequence_activate(
					*flat_vals.begin(),
					new_inner_state_vals,
					local_state_vals,
					input_vals,
					new_outer_state_vals,
					state_network_history);
				history->state_network_histories[f_index][i_index] = state_network_history;

				new_inner_state_vals[i_index] += this->test_state_networks[f_index][i_index]->output->acti_vals[0];
			}
			for (int l_index = 0; l_index < this->num_sequence_local_states; l_index++) {
				int state_index = this->sum_inner_inputs
					+ this->test_num_new_inner_states
					+ l_index;
				StateNetworkHistory* state_network_history = new StateNetworkHistory(this->test_state_networks[f_index][state_index]);
				this->test_state_networks[f_index][state_index]->new_sequence_activate(
					*flat_vals.begin(),
					new_inner_state_vals,
					local_state_vals,
					input_vals,
					new_outer_state_vals,
					state_network_history);
				history->state_network_histories[f_index][state_index] = state_network_history;

				local_state_vals[l_index] += this->test_state_networks[f_index][state_index]->output->acti_vals[0];
			}
			for (int i_index = 0; i_index < this->num_sequence_input_states; i_index++) {
				int state_index = this->sum_inner_inputs
					+ this->test_num_new_inner_states
					+ this->num_sequence_local_states
					+ i_index;
				StateNetworkHistory* state_network_history = new StateNetworkHistory(this->test_state_networks[f_index][state_index]);
				this->test_state_networks[f_index][state_index]->new_sequence_activate(
					*flat_vals.begin(),
					new_inner_state_vals,
					local_state_vals,
					input_vals,
					new_outer_state_vals,
					state_network_history);
				history->state_network_histories[f_index][state_index] = state_network_history;

				input_vals[i_index] += this->test_state_networks[f_index][state_index]->output->acti_vals[0];
			}
			for (int o_index = 0; o_index < this->test_num_new_outer_states; o_index++) {
				int state_index = this->sum_inner_inputs
					+ this->test_num_new_inner_states
					+ this->num_sequence_local_states
					+ this->num_sequence_input_states
					+ o_index;
				StateNetworkHistory* state_network_history = new StateNetworkHistory(this->test_state_networks[f_index][state_index]);
				this->test_state_networks[f_index][state_index]->new_sequence_activate(
					*flat_vals.begin(),
					new_inner_state_vals,
					local_state_vals,
					input_vals,
					new_outer_state_vals,
					state_network_history);
				history->state_network_histories[f_index][state_index] = state_network_history;

				new_outer_state_vals[o_index] += this->test_state_networks[f_index][state_index]->output->acti_vals[0];
			}

			flat_vals.erase(flat_vals.begin());
		}

		StateNetworkHistory* score_network_history = new StateNetworkHistory(this->test_score_networks[f_index]);
		this->test_score_networks[f_index]->new_sequence_activate(
			new_inner_state_vals,
			local_state_vals,
			input_vals,
			new_outer_state_vals,
			score_network_history);
		history->score_network_updates[f_index] = this->test_score_networks[f_index]->output->acti_vals[0];
		history->score_network_histories[f_index] = score_network_history;
		predicted_score += scale_factor*this->test_score_networks[f_index]->output->acti_vals[0];
	}

	if (run_helper.is_recursive) {
		this->is_recursive++;
	}
}

void Fold::explore_on_path_backprop(vector<double>& local_state_errors,
									vector<double>& input_errors,
									double target_val,
									double final_misguess,
									double& predicted_score,
									double& scale_factor,
									FoldHistory* history) {
	this->test_average_score = 0.9999*this->test_average_score + 0.0001*target_val;
	double score_variance = (this->test_average_score - target_val)*(this->test_average_score - target_val);
	this->test_score_variance = 0.9999*this->test_score_variance + 0.0001*score_variance;

	this->test_average_misguess = 0.9999*this->test_average_misguess + 0.0001*final_misguess;
	double misguess_variance = (this->test_average_misguess - final_misguess)*(this->test_average_misguess - final_misguess);
	this->test_misguess_variance = 0.9999*this->test_misguess_variance + 0.0001*misguess_variance;

	this->sum_error += abs(target_val-predicted_score);

	vector<double> new_inner_state_errors(this->sum_inner_inputs+this->test_num_new_inner_states, 0.0);
	vector<double> new_outer_state_errors(this->test_num_new_outer_states, 0.0);

	double target_max_update;
	if (this->state_iter <= 100000) {
		target_max_update = 0.05;
	} else if (this->state_iter <= 400000) {
		target_max_update = 0.01;
	} else {
		target_max_update = 0.002;
	}

	for (int f_index = this->sequence_length-1; f_index >= 0; f_index--) {
		this->test_score_networks[f_index]->new_sequence_backprop(
			target_val - predicted_score,
			new_inner_state_errors,
			local_state_errors,
			input_errors,
			new_outer_state_errors,
			target_max_update,
			history->score_network_histories[f_index]);

		predicted_score -= scale_factor*history->score_network_updates[f_index];

		if (this->is_inner_scope[f_index]) {
			for (int o_index = this->test_num_new_outer_states-1; o_index >= 0; o_index--) {
				int state_index = this->sum_inner_inputs
					+ this->test_num_new_inner_states
					+ this->num_sequence_local_states
					+ this->num_sequence_input_states
					+ o_index;
				this->test_state_networks[f_index][state_index]->new_sequence_backprop(
					new_outer_state_errors[o_index],
					new_inner_state_errors,
					local_state_errors,
					input_errors,
					new_outer_state_errors,
					target_max_update,
					history->state_network_histories[f_index][state_index]);
			}
			for (int i_index = this->num_sequence_input_states-1; i_index >= 0; i_index--) {
				int state_index = this->sum_inner_inputs
					+ this->test_num_new_inner_states
					+ this->num_sequence_local_states
					+ i_index;
				this->test_state_networks[f_index][state_index]->new_sequence_backprop(
					input_errors[i_index],
					new_inner_state_errors,
					local_state_errors,
					input_errors,
					new_outer_state_errors,
					target_max_update,
					history->state_network_histories[f_index][state_index]);
			}
			for (int l_index = this->num_sequence_local_states-1; l_index >= 0; l_index--) {
				int state_index = this->sum_inner_inputs
					+ this->test_num_new_inner_states
					+ l_index;
				this->test_state_networks[f_index][state_index]->new_sequence_backprop(
					local_state_errors[l_index],
					new_inner_state_errors,
					local_state_errors,
					input_errors,
					new_outer_state_errors,
					target_max_update,
					history->state_network_histories[f_index][state_index]);
			}
			for (int i_index = this->sum_inner_inputs+this->test_num_new_inner_states-1;
					i_index >= this->inner_input_start_indexes[f_index] + this->num_inner_inputs[f_index]; i_index--) {
				this->test_state_networks[f_index][i_index]->new_sequence_backprop(
					new_inner_state_errors[i_index],
					new_inner_state_errors,
					local_state_errors,
					input_errors,
					new_outer_state_errors,
					target_max_update,
					history->state_network_histories[f_index][i_index]);
			}

			Scope* inner_scope = solution->scopes[this->existing_scope_ids[f_index]];
			vector<double> inner_input_errors(new_inner_state_errors.begin() + this->inner_input_start_indexes[f_index],
				new_inner_state_errors.begin() + this->inner_input_start_indexes[f_index] + this->num_inner_inputs[f_index]);
			int num_input_states_diff = inner_scope->num_input_states - this->num_inner_inputs[f_index];
			inner_input_errors.insert(inner_input_errors.end(), num_input_states_diff, 0.0);
			inner_scope->existing_explore_backprop(inner_input_errors,
												   target_val,
												   predicted_score,
												   scale_factor,
												   history->inner_scope_histories[f_index]);
			for (int i_index = 0; i_index < this->num_inner_inputs[f_index]; i_index++) {
				new_inner_state_errors[this->inner_input_start_indexes[f_index] + i_index] = inner_input_errors[i_index];
			}

			for (int i_index = this->inner_input_start_indexes[f_index]+this->num_inner_inputs[f_index]-1; i_index >= 0; i_index--) {
				this->test_state_networks[f_index][i_index]->new_sequence_backprop(
					new_inner_state_errors[i_index],
					new_inner_state_errors,
					local_state_errors,
					input_errors,
					new_outer_state_errors,
					target_max_update,
					history->state_network_histories[f_index][i_index]);
			}
		} else {
			for (int o_index = this->test_num_new_outer_states-1; o_index >= 0; o_index--) {
				int state_index = this->sum_inner_inputs
					+ this->test_num_new_inner_states
					+ this->num_sequence_local_states
					+ this->num_sequence_input_states
					+ o_index;
				this->test_state_networks[f_index][state_index]->new_sequence_backprop(
					new_outer_state_errors[o_index],
					new_inner_state_errors,
					local_state_errors,
					input_errors,
					new_outer_state_errors,
					target_max_update,
					history->state_network_histories[f_index][state_index]);
			}
			for (int i_index = this->num_sequence_input_states-1; i_index >= 0; i_index--) {
				int state_index = this->sum_inner_inputs
					+ this->test_num_new_inner_states
					+ this->num_sequence_local_states
					+ i_index;
				this->test_state_networks[f_index][state_index]->new_sequence_backprop(
					input_errors[i_index],
					new_inner_state_errors,
					local_state_errors,
					input_errors,
					new_outer_state_errors,
					target_max_update,
					history->state_network_histories[f_index][state_index]);
			}
			for (int l_index = this->num_sequence_local_states-1; l_index >= 0; l_index--) {
				int state_index = this->sum_inner_inputs
					+ this->test_num_new_inner_states
					+ l_index;
				this->test_state_networks[f_index][state_index]->new_sequence_backprop(
					local_state_errors[l_index],
					new_inner_state_errors,
					local_state_errors,
					input_errors,
					new_outer_state_errors,
					target_max_update,
					history->state_network_histories[f_index][state_index]);
			}
			for (int i_index = this->sum_inner_inputs+this->test_num_new_inner_states-1; i_index >= 0; i_index--) {
				this->test_state_networks[f_index][i_index]->new_sequence_backprop(
					new_inner_state_errors[i_index],
					new_inner_state_errors,
					local_state_errors,
					input_errors,
					new_outer_state_errors,
					target_max_update,
					history->state_network_histories[f_index][i_index]);
			}
		}
	}

	this->test_starting_score_network->new_outer_backprop(
		target_val - predicted_score,
		new_outer_state_errors,
		target_max_update,
		history->starting_score_network_history);

	predicted_score -= scale_factor*history->starting_score_update;

	for (int n_index = history->outer_state_network_histories.size()-1; n_index >= 0; n_index--) {
		for (int o_index = this->test_num_new_outer_states-1; o_index >= 0; o_index--) {
			StateNetwork* state_network = history->outer_state_network_histories[n_index][o_index]->network;
			state_network->new_outer_backprop(
				new_outer_state_errors[o_index],
				new_outer_state_errors,
				target_max_update,
				history->outer_state_network_histories[n_index][o_index]);
		}
	}

	explore_increment();
}

void Fold::explore_increment() {
	this->state_iter++;

	if (this->state == FOLD_STATE_EXPLORE) {
		if (this->state_iter == 500000) {
			explore_end();
		} else {
			if (this->state_iter%10000 == 0) {
				cout << "this->state_iter: " << this->state_iter << endl;
				cout << "this->sum_error: " << this->sum_error << endl;
				cout << endl;
				this->sum_error = 0.0;
			}
		}
	} else if (this->state == FOLD_STATE_ADD_INNER_STATE) {
		if (this->state_iter == 500000) {
			add_inner_state_end();
		} else {
			if (this->state_iter%10000 == 0) {
				cout << "this->state_iter: " << this->state_iter << endl;
				cout << "this->sum_error: " << this->sum_error << endl;
				cout << endl;
				this->sum_error = 0.0;
			}
		}
	} else {
		// this->state == FOLD_STATE_ADD_OUTER_STATE
		if (this->state_iter == 500000) {
			add_outer_state_end();
		} else {
			if (this->state_iter%10000 == 0) {
				cout << "this->state_iter: " << this->state_iter << endl;
				cout << "this->sum_error: " << this->sum_error << endl;
				cout << endl;
				this->sum_error = 0.0;
			}
		}
	}
}
