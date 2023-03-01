#include "fold.h"

#include <cmath>
#include <iostream>

using namespace std;

Fold::Fold(int sequence_length) {
	this->sequence_length = sequence_length;

	this->curr_num_states = 1;
	this->curr_state_networks = vector<vector<Network*>>(this->sequence_length);
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		this->curr_state_networks[f_index] = vector<Network*>{new Network(2, 20, 1)};
	}
	this->curr_score_networks = vector<Network*>(this->sequence_length);
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		this->curr_score_networks[f_index] = new Network(1, 20, 1);
	}

	this->curr_average_misguess = 0.0;
	this->curr_misguess_variance = 0.0;

	this->curr_step_impacts = vector<double>(this->sequence_length, 0.0);

	this->state = STATE_EXPLORE;
	this->state_iter = 0;
	this->sum_error = 0.0;
}

Fold::~Fold() {
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		for (int s_index = 0; s_index < this->curr_num_states; s_index++) {
			if (this->curr_state_networks[f_index][s_index] != NULL) {
				delete this->curr_state_networks[f_index][s_index];
			}
		}

		delete this->curr_score_networks[f_index];
	}
}

double Fold::explore_score_activate(vector<double>& local_state_vals,
									vector<double>& input_vals,
									vector<int>& context_iter,
									vector<ContextHistory*> context_histories,
									RunHelper& run_helper,
									FoldHistory* history) {
	vector<double> new_outer_state_vals(this->curr_num_new_outer_states, 0.0);

	int starting_iter = context_iter[context_iter.size() - this->scope_context.size()];
	for (int n_index = starting_iter; n_index < (int)context_histories.size(); n_index++) {
		map<int, vector<vector<Network*>>>::iterator it = this->curr_outer_state_networks.find(context_histories[n_index]->scope_id);
		if (it == this->curr_outer_state_networks.end()) {
			it = this->curr_outer_state_networks.insert({context_histories[n_index]->scope_id, vector<vector<Network*>>()}).first;
		}

		if (context_histories[n_index]->node_id >= it->second.size()) {
			int size_diff = solution->scopes[context_histories[n_index]->scope_id]->nodes.size() - it->second.size();
			it->second.insert(it->second.begin(), size_diff, vector<Network*>());
		}

		if (it->second[context_histories[n_index]->node_id].size() == 0) {
			for (int s_index = 0; s_index < this->curr_num_new_outer_states; s_index++) {
				it->second[context_histories[n_index]->node_id].push_back(new StateNetwork(
					1,
					solution->scopes[context_histories[n_index]->scope_id]->num_local_states,
					solution->scopes[context_histories[n_index]->scope_id]->num_input_states,
					0,
					this->curr_num_new_outer_states,
					20));
			}
		}

		history->outer_state_network_histories.push_back(vector<StateNetworkHistory*>());
		for (int s_index = 0; s_index < this->curr_num_new_outer_states; s_index++) {
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

	StateNetworkHistory* starting_score_network_history = new StateNetworkHistory(this->curr_starting_score_network);
	this->curr_starting_score_network->new_outer_activate(
		local_state_vals,
		input_vals,
		new_outer_state_vals,
		starting_score_network_history);
	history->starting_score_update = this->curr_starting_score_network->output->acti_vals[0];
	history->starting_score_network_history = starting_score_network_history;

	history->new_outer_state_vals = new_outer_state_vals;

	return this->curr_starting_score_network->output->acti_vals[0];
}

void Fold::explore_sequence_activate(vector<double>& local_state_vals,
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
			Scope* inner_scope = solution->scopes[this->existing_scope_ids[f_index]];
			for (int i_index = 0; i_index < this->inner_input_start_indexes[f_index] + inner_scope->num_input_states; i_index++) {
				StateNetworkHistory* state_network_history = new StateNetworkHistory(this->curr_state_networks[f_index][i_index]);
				// don't check obs for is_inner_scope
				this->curr_state_networks[f_index][i_index]->new_sequence_activate(
					new_inner_state_vals,
					local_state_vals,
					input_vals,
					new_outer_state_vals,
					state_network_history);
				history->state_network_histories[f_index][i_index] = state_network_history;

				new_inner_state_vals[i_index] += this->curr_state_networks[f_index][i_index]->output->acti_vals[0];
			}

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

			// update back state so have chance to compress front
			for (int i_index = this->inner_input_start_indexes[f_index] + inner_scope->num_input_states;
					i_index < this->sum_inner_inputs + this->curr_num_new_inner_states; i_index++) {
				StateNetworkHistory* state_network_history = new StateNetworkHistory(this->curr_state_networks[f_index][i_index]);
				this->curr_state_networks[f_index][i_index]->new_sequence_activate(
					new_inner_state_vals,
					local_state_vals,
					input_vals,
					new_outer_state_vals,
					state_network_history);
				history->state_network_histories[f_index][i_index] = state_network_history;

				new_inner_state_vals[i_index] += this->curr_state_networks[f_index][i_index]->output->acti_vals[0];
			}
			for (int l_index = 0; l_index < this->num_local_states; l_index++) {
				int state_network_index = this->sum_inner_inputs
					+ this->curr_num_new_inner_states
					+ l_index;
				StateNetworkHistory* state_network_history = new StateNetworkHistory(this->curr_state_networks[f_index][state_network_index]);
				this->curr_state_networks[f_index][state_network_index]->new_sequence_activate(
					new_inner_state_vals,
					local_state_vals,
					input_vals,
					new_outer_state_vals,
					state_network_history);
				history->state_network_histories[f_index][state_network_index] = state_network_history;

				local_state_vals[l_index] += this->curr_state_networks[f_index][state_network_index]->output->acti_vals[0];
			}
			for (int i_index = 0; i_index < this->num_input_states; i_index++) {
				int state_network_index = this->sum_inner_inputs
					+ this->curr_num_new_inner_states
					+ this->num_local_states
					+ i_index;
				StateNetworkHistory* state_network_history = new StateNetworkHistory(this->curr_state_networks[f_index][state_network_index]);
				this->curr_state_networks[f_index][state_network_index]->new_sequence_activate(
					new_inner_state_vals,
					local_state_vals,
					input_vals,
					new_outer_state_vals,
					state_network_history);
				history->state_network_histories[f_index][state_network_index] = state_network_history;

				input_vals[i_index] += this->curr_state_networks[f_index][state_network_index]->output->acti_vals[0];
			}
			for (int o_index = 0; o_index < this->curr_num_new_outer_states; o_index++) {
				int state_network_index = this->sum_inner_inputs
					+ this->curr_num_new_inner_states
					+ this->num_local_states
					+ this->num_input_states
					+ o_index;
				StateNetworkHistory* state_network_history = new StateNetworkHistory(this->curr_state_networks[f_index][state_network_index]);
				this->curr_state_networks[f_index][state_network_index]->new_sequence_activate(
					new_inner_state_vals,
					local_state_vals,
					input_vals,
					new_outer_state_vals,
					state_network_history);
				history->state_network_histories[f_index][state_network_index] = state_network_history;

				new_outer_state_vals[o_index] += this->curr_state_networks[f_index][state_network_index]->output->acti_vals[0];
			}
		} else {
			for (int i_index = 0; i_index < this->sum_inner_inputs + this->curr_num_new_inner_states; i_index++) {
				StateNetworkHistory* state_network_history = new StateNetworkHistory(this->curr_state_networks[f_index][i_index]);
				this->curr_state_networks[f_index][i_index]->new_sequence_activate(
					*flat_vals.begin(),
					new_inner_state_vals,
					local_state_vals,
					input_vals,
					new_outer_state_vals,
					state_network_history);
				history->state_network_histories[f_index][i_index] = state_network_history;

				new_inner_state_vals[i_index] += this->curr_state_networks[f_index][i_index]->output->acti_vals[0];
			}
			for (int l_index = 0; l_index < this->num_local_states; l_index++) {
				int state_network_index = this->sum_inner_inputs
					+ this->curr_num_new_inner_states
					+ l_index;
				StateNetworkHistory* state_network_history = new StateNetworkHistory(this->curr_state_networks[f_index][state_network_index]);
				this->curr_state_networks[f_index][state_network_index]->new_sequence_activate(
					*flat_vals.begin(),
					new_inner_state_vals,
					local_state_vals,
					input_vals,
					new_outer_state_vals,
					state_network_history);
				history->state_network_histories[f_index][state_network_index] = state_network_history;

				local_state_vals[l_index] += this->curr_state_networks[f_index][state_network_index]->output->acti_vals[0];
			}
			for (int i_index = 0; i_index < this->num_input_states; i_index++) {
				int state_network_index = this->sum_inner_inputs
					+ this->curr_num_new_inner_states
					+ this->num_local_states
					+ i_index;
				StateNetworkHistory* state_network_history = new StateNetworkHistory(this->curr_state_networks[f_index][state_network_index]);
				this->curr_state_networks[f_index][state_network_index]->new_sequence_activate(
					*flat_vals.begin(),
					new_inner_state_vals,
					local_state_vals,
					input_vals,
					new_outer_state_vals,
					state_network_history);
				history->state_network_histories[f_index][state_network_index] = state_network_history;

				input_vals[i_index] += this->curr_state_networks[f_index][state_network_index]->output->acti_vals[0];
			}
			for (int o_index = 0; o_index < this->curr_num_new_outer_states; o_index++) {
				int state_network_index = this->sum_inner_inputs
					+ this->curr_num_new_inner_states
					+ this->num_local_states
					+ this->num_input_states
					+ o_index;
				StateNetworkHistory* state_network_history = new StateNetworkHistory(this->curr_state_networks[f_index][state_network_index]);
				this->curr_state_networks[f_index][state_network_index]->new_sequence_activate(
					*flat_vals.begin(),
					new_inner_state_vals,
					local_state_vals,
					input_vals,
					new_outer_state_vals,
					state_network_history);
				history->state_network_histories[f_index][state_network_index] = state_network_history;

				new_outer_state_vals[o_index] += this->curr_state_networks[f_index][state_network_index]->output->acti_vals[0];
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

void Fold::explore_backprop(vector<double>& local_state_errors,
							vector<double>& input_errors,
							double target_val,
							double final_misguess,
							double& predicted_score,
							double& scale_factor,
							FoldHistory* history) {
	this->curr_average_misguess = 0.9999*this->curr_average_misguess + 0.0001*final_misguess;
	double misguess_variance = (this->curr_average_misguess - final_misguess)*(this->curr_average_misguess - final_misguess);
	this->curr_misguess_variance = 0.9999*this->curr_misguess_variance + 0.0001*misguess_variance;

	this->sum_error += abs(target_val-predicted_score);

	vector<double> new_inner_state_errors(this->curr_num_new_inner_states, 0.0);
	vector<double> new_outer_state_errors(this->curr_num_new_outer_states, 0.0);
	int num_total_states = fold->sum_inner_inputs
		+ fold->curr_num_new_inner_states
		+ fold->num_local_states
		+ fold->num_input_states
		+ fold->curr_num_new_outer_states;

	double target_max_update;
	if (this->state_iter <= 100000) {
		target_max_update = 0.05;
	} else if (this->state_iter <= 400000) {
		target_max_update = 0.01;
	} else {
		target_max_update = 0.002;
	}

	for (int f_index = this->sequence_length-1; f_index >= 0; f_index--) {
		this->curr_score_networks[f_index]->new_sequence_backprop(
			target_val - predicted_score,
			new_inner_state_errors,
			local_state_errors,
			input_errors,
			new_outer_state_errors,
			target_max_update,
			history->score_network_histories[f_index]);

		this->curr_step_impacts[f_index] = 0.9999*this->curr_step_impacts[f_index]
			+ 0.0001*abs(scale_factor*history->score_network_updates[f_index]);

		predicted_score -= scale_factor*history->score_network_updates[f_index];

		if (this->is_inner_scope[f_index]) {
			Scope* inner_scope = solution->scopes[this->existing_scope_ids[f_index]];

			for (int o_index = this->curr_num_new_outer_states-1; o_index >= 0; o_index--) {
				int state_network_index = this->sum_inner_inputs
					+ this->curr_num_new_inner_states
					+ this->num_local_states
					+ this->num_input_states
					+ o_index;
				this->curr_state_networks[f_index][state_network_index]->new_sequence_backprop(
					new_outer_state_errors[o_index],
					new_inner_state_errors,
					local_state_errors,
					input_errors,
					new_outer_state_errors,
					target_max_update,
					history->state_network_histories[f_index][state_network_index]);
			}
			for (int i_index = this->num_input_states-1; i_index >= 0; i_index--) {
				int state_network_index = this->sum_inner_inputs
					+ this->curr_num_new_inner_states
					+ this->num_local_states
					+ i_index;
				this->curr_state_networks[f_index][state_network_index]->new_sequence_backprop(
					input_errors[i_index],
					new_inner_state_errors,
					local_state_errors,
					input_errors,
					new_outer_state_errors,
					target_max_update,
					history->state_network_histories[f_index][state_network_index]);
			}
			for (int l_index = this->num_local_states-1; l_index >= 0; l_index--) {
				int state_network_index = this->sum_inner_inputs
					+ this->curr_num_new_inner_states
					+ l_index;
				this->curr_state_networks[f_index][state_network_index]->new_sequence_backprop(
					local_state_errors[l_index],
					new_inner_state_errors,
					local_state_errors,
					input_errors,
					new_outer_state_errors,
					target_max_update,
					history->state_network_histories[f_index][state_network_index]);
			}
			for (int i_index = this->sum_inner_inputs+this->curr_num_new_inner_states-1;
					i_index >= this->inner_input_start_indexes[f_index] + inner_scope->num_input_states; i_index--) {
				this->curr_state_networks[f_index][i_index]->new_sequence_backprop(
					new_inner_state_errors[i_index],
					new_inner_state_errors,
					local_state_errors,
					input_errors,
					new_outer_state_errors,
					target_max_update,
					history->state_network_histories[f_index][i_index]);
			}

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

			for (int i_index = this->inner_input_start_indexes[f_index]+inner_scope->num_input_states-1; i_index >= 0; i_index--) {
				this->curr_state_networks[f_index][i_index]->new_sequence_backprop(
					new_inner_state_errors[i_index],
					new_inner_state_errors,
					local_state_errors,
					input_errors,
					new_outer_state_errors,
					target_max_update,
					history->state_network_histories[f_index][i_index]);
			}
		} else {
			for (int o_index = this->curr_num_new_outer_states-1; o_index >= 0; o_index--) {
				int state_network_index = this->sum_inner_inputs
					+ this->curr_num_new_inner_states
					+ this->num_local_states
					+ this->num_input_states
					+ o_index;
				this->curr_state_networks[f_index][state_network_index]->new_sequence_backprop(
					new_outer_state_errors[o_index],
					new_inner_state_errors,
					local_state_errors,
					input_errors,
					new_outer_state_errors,
					target_max_update,
					history->state_network_histories[f_index][state_network_index]);
			}
			for (int i_index = this->num_input_states-1; i_index >= 0; i_index--) {
				int state_network_index = this->sum_inner_inputs
					+ this->curr_num_new_inner_states
					+ this->num_local_states
					+ i_index;
				this->curr_state_networks[f_index][state_network_index]->new_sequence_backprop(
					input_errors[i_index],
					new_inner_state_errors,
					local_state_errors,
					input_errors,
					new_outer_state_errors,
					target_max_update,
					history->state_network_histories[f_index][state_network_index]);
			}
			for (int l_index = this->num_local_states-1; l_index >= 0; l_index--) {
				int state_network_index = this->sum_inner_inputs
					+ this->curr_num_new_inner_states
					+ l_index;
				this->curr_state_networks[f_index][state_network_index]->new_sequence_backprop(
					local_state_errors[l_index],
					new_inner_state_errors,
					local_state_errors,
					input_errors,
					new_outer_state_errors,
					target_max_update,
					history->state_network_histories[f_index][state_network_index]);
			}
			for (int i_index = this->sum_inner_inputs+this->curr_num_new_inner_states-1; i_index >= 0; i_index--) {
				this->curr_state_networks[f_index][i_index]->new_sequence_backprop(
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

	this->curr_starting_score_network->new_outer_backprop(
		target_val - predicted_score,
		new_outer_state_errors,
		target_max_update,
		history->starting_score_network_history);

	predicted_score -= scale_factor*history->starting_score_update;

	for (int n_index = history->outer_state_network_histories.size()-1; n_index >= 0; n_index--) {
		for (int o_index = this->curr_num_new_outer_states-1; o_index >= 0; o_index--) {
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

	// Note: distant to-do -- eventually, swap to using seeds instead of new problem instances every time
	if (this->state_iter == 500000) {
		cout << "STATE_EXPLORE_DONE" << endl;
		this->state = STATE_EXPLORE_DONE;
	} else {
		if (this->state_iter%10000 == 0) {
			cout << "this->state_iter: " << this->state_iter << endl;
			cout << "this->sum_error: " << this->sum_error << endl;
			cout << endl;
			this->sum_error = 0.0;
		}
	}
}

void Fold::explore_to_fold() {
	this->test_num_states = this->curr_num_states+1;

	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		this->test_state_networks.push_back(vector<Network*>());
		this->test_state_networks[f_index].push_back(new Network(this->curr_state_networks[f_index][0]));
		int new_input_size = 1 + 2;	// flat size always 1 for now
		this->test_state_networks[f_index].push_back(new Network(new_input_size,
																 20,
																 1));

		this->test_score_networks.push_back(new Network(this->curr_score_networks[f_index]));
		this->test_score_networks[f_index]->add_input();
	}

	this->test_average_misguess = 0.0;
	this->test_misguess_variance = 0.0;

	this->test_step_impacts = vector<double>(this->sequence_length, 0.0);

	cout << "STATE_ADD_STATE " << this->test_num_states << endl;

	this->state = STATE_ADD_STATE;
	this->state_iter = 0;
	this->sum_error = 0.0;
}

// void Fold::add_activate(vector<vector<double>>& flat_vals,
// 						double& predicted_score) {
// 	vector<double> state_vals(this->test_num_states, 0.0);

// 	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
// 		for (int s_index = 0; s_index < this->test_num_states; s_index++) {
// 			vector<double> state_network_input = *flat_vals.begin();
// 			for (int i_index = 0; i_index < s_index+1; i_index++) {
// 				state_network_input.push_back(state_vals[i_index]);
// 			}
// 			this->test_state_networks[f_index][s_index]->activate(state_network_input);
// 			state_vals[s_index] += this->test_state_networks[f_index][s_index]->output->acti_vals[0];
// 		}
// 		flat_vals.erase(flat_vals.begin());

// 		this->test_score_networks[f_index]->activate(state_vals);
// 		predicted_score += this->test_score_networks[f_index]->output->acti_vals[0];
// 	}
// }

// void Fold::add_backprop(double target_val,
// 						double final_misguess,
// 						double& predicted_score) {
// 	this->test_average_misguess = 0.9999*this->test_average_misguess + 0.0001*final_misguess;
// 	double misguess_variance = (this->test_average_misguess - final_misguess)*(this->test_average_misguess - final_misguess);
// 	this->test_misguess_variance = 0.9999*this->test_misguess_variance + 0.0001*misguess_variance;

// 	this->sum_error += abs(target_val-predicted_score);

// 	vector<double> state_errors(this->test_num_states, 0.0);

// 	for (int f_index = this->sequence_length-1; f_index >= 0; f_index--) {
// 		vector<double> score_network_errors{target_val - predicted_score};
// 		if (this->state_iter <= 400000) {
// 			this->test_score_networks[f_index]->backprop(score_network_errors, 0.01);
// 		} else {
// 			this->test_score_networks[f_index]->backprop(score_network_errors, 0.002);
// 		}
// 		for (int i_index = 0; i_index < this->test_num_states; i_index++) {
// 			state_errors[i_index] += this->test_score_networks[f_index]->input->errors[i_index];
// 			this->test_score_networks[f_index]->input->errors[i_index] = 0.0;
// 		}

// 		this->test_step_impacts[f_index] = 0.9999*this->test_step_impacts[f_index]
// 			+ 0.0001*abs(this->test_score_networks[f_index]->output->acti_vals[0]);

// 		predicted_score -= this->test_score_networks[f_index]->output->acti_vals[0];

// 		for (int s_index = this->test_num_states-1; s_index >= 0; s_index--) {
// 			vector<double> state_network_output_errors{state_errors[s_index]};
// 			if (s_index == this->test_num_states-1 && this->state_iter <= 100000) {
// 				this->test_state_networks[f_index][s_index]->backprop(state_network_output_errors, 0.05);
// 			} else if (this->state_iter <= 400000) {
// 				this->test_state_networks[f_index][s_index]->backprop(state_network_output_errors, 0.01);
// 			} else {
// 				this->test_state_networks[f_index][s_index]->backprop(state_network_output_errors, 0.002);
// 			}
// 			for (int i_index = 0; i_index < s_index+1; i_index++) {
// 				state_errors[i_index] += this->test_state_networks[f_index][s_index]->input->errors[1+i_index];
// 				this->test_state_networks[f_index][s_index]->input->errors[1+i_index] = 0.0;
// 			}
// 		}
// 	}

// 	add_increment();
// }

// void Fold::add_increment() {
// 	this->state_iter++;

// 	if (this->state_iter == 500000) {
// 		double misguess_standard_deviation = sqrt(this->curr_misguess_variance);
// 		cout << "misguess_standard_deviation: " << misguess_standard_deviation << endl;

// 		cout << "this->curr_average_misguess: " << this->curr_average_misguess << endl;
// 		cout << "this->test_average_misguess: " << this->test_average_misguess << endl;

// 		double misguess_improvement = this->curr_average_misguess - this->test_average_misguess;
// 		cout << "misguess_improvement: " << misguess_improvement << endl;

// 		// 0.0001 rolling average variance approx. equal to 20000 average variance (?)
// 		double misguess_improvement_t_value = misguess_improvement
// 			/ (misguess_standard_deviation / sqrt(20000));
// 		cout << "misguess_improvement_t_value: " << misguess_improvement_t_value << endl;

// 		// if (misguess_improvement_t_value > 2.326) {	// >99%
// 		if (misguess_improvement_t_value > 2.326 && this->test_num_states < 3) {	// >99%
// 			cout << "STATE_ADD_STATE " << this->test_num_states << " success" << endl;
// 			for (int f_index = 0; f_index < this->sequence_length; f_index++) {
// 				for (int s_index = 0; s_index < this->curr_num_states; s_index++) {
// 					delete this->curr_state_networks[f_index][s_index];
// 				}

// 				delete this->curr_score_networks[f_index];
// 			}

// 			this->curr_num_states = this->test_num_states;
// 			this->curr_state_networks = this->test_state_networks;
// 			this->curr_score_networks = this->test_score_networks;
// 			this->curr_average_misguess = this->test_average_misguess;
// 			this->curr_misguess_variance = this->test_misguess_variance;
// 			this->curr_step_impacts = this->test_step_impacts;

// 			this->test_num_states = this->curr_num_states+1;
	
// 			for (int f_index = 0; f_index < this->sequence_length; f_index++) {
// 				for (int s_index = 0; s_index < this->curr_num_states; s_index++) {
// 					this->test_state_networks[f_index][s_index] = new Network(this->curr_state_networks[f_index][s_index]);
// 				}
// 				int new_input_size = 1 + this->test_num_states;	// flat size always 1 for now
// 				this->test_state_networks[f_index].push_back(new Network(new_input_size,
// 																		 20,
// 																		 1));

// 				this->test_score_networks[f_index] = new Network(this->curr_score_networks[f_index]);
// 				this->test_score_networks[f_index]->add_input();
// 			}

// 			this->test_average_misguess = 0.0;
// 			this->test_misguess_variance = 0.0;

// 			this->test_step_impacts = vector<double>(this->sequence_length, 0.0);

// 			cout << "STATE_ADD_STATE " << this->test_num_states << endl;

// 			// no change to this->state
// 			this->state_iter = 0;
// 			this->sum_error = 0.0;
// 		} else {
// 			cout << "STATE_ADD_STATE " << this->test_num_states << " failure" << endl;
// 			for (int f_index = 0; f_index < this->sequence_length; f_index++) {
// 				for (int s_index = 0; s_index < this->test_num_states; s_index++) {
// 					delete this->test_state_networks[f_index][s_index];
// 				}
// 				this->test_state_networks[f_index].pop_back();	// set test_state_networks size back to match curr_state_networks

// 				delete this->test_score_networks[f_index];
// 			}

// 			this->state = STATE_ADD_DONE;
// 		}
// 	} else {
// 		if (this->state_iter%10000 == 0) {
// 			cout << "this->state_iter: " << this->state_iter << endl;
// 			cout << "this->sum_error: " << this->sum_error << endl;
// 			cout << endl;
// 			this->sum_error = 0.0;
// 		}
// 	}
// }

// void Fold::add_to_clean() {
// 	this->state = STATE_REMOVE_NETWORK;
// 	this->clean_step_index = 0;
// 	this->clean_state_index = 0;

// 	cout << "STATE_REMOVE_NETWORK " << this->clean_step_index << " " << this->clean_state_index << endl;

// 	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
// 		this->curr_state_networks_not_needed.push_back(vector<bool>(this->curr_num_states, false));
// 		this->test_state_networks_not_needed.push_back(vector<bool>(this->curr_num_states, false));
// 		this->curr_state_not_needed_locally.push_back(vector<bool>(this->curr_num_states, false));
// 		this->test_state_not_needed_locally.push_back(vector<bool>(this->curr_num_states, false));
// 		this->curr_num_states_cleared.push_back(0);
// 		this->test_num_states_cleared.push_back(0);
// 	}

// 	this->test_state_networks_not_needed[0][this->curr_num_states-1] = true;
// 	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
// 		for (int s_index = 0; s_index < this->curr_num_states; s_index++) {
// 			if (!this->test_state_networks_not_needed[f_index][s_index]) {
// 				this->test_state_networks[f_index][s_index] = new Network(this->curr_state_networks[f_index][s_index]);
// 			} else {
// 				this->test_state_networks[f_index][s_index] = NULL;
// 			}
// 		}

// 		this->test_score_networks[f_index] = new Network(this->curr_score_networks[f_index]);
// 	}

// 	this->state_iter = 0;
// 	this->sum_error = 0.0;
// }

// void Fold::clean_activate(vector<vector<double>>& flat_vals,
// 						  double& predicted_score) {
// 	vector<double> curr_state_vals(this->curr_num_states, 0.0);
// 	vector<double> test_state_vals(this->curr_num_states, 0.0);

// 	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
// 		for (int s_index = 0; s_index < this->curr_num_states; s_index++) {
// 			if (!this->curr_state_networks_not_needed[f_index][s_index]) {
// 				vector<double> state_network_input = *flat_vals.begin();
// 				for (int i_index = 0; i_index < s_index+1; i_index++) {
// 					if (this->curr_state_not_needed_locally[f_index][i_index]) {
// 						state_network_input.push_back(0.0);
// 					} else {
// 						state_network_input.push_back(curr_state_vals[i_index]);
// 					}
// 				}
// 				this->curr_state_networks[f_index][s_index]->activate(state_network_input);
// 				curr_state_vals[s_index] += this->curr_state_networks[f_index][s_index]->output->acti_vals[0];
// 			}
// 		}
// 		for (int s_index = 0; s_index < this->curr_num_states; s_index++) {
// 			if (!this->test_state_networks_not_needed[f_index][s_index]) {
// 				vector<double> state_network_input = *flat_vals.begin();
// 				for (int i_index = 0; i_index < s_index+1; i_index++) {
// 					if (this->test_state_not_needed_locally[f_index][i_index]) {
// 						state_network_input.push_back(0.0);
// 					} else {
// 						state_network_input.push_back(test_state_vals[i_index]);
// 					}
// 				}
// 				this->test_state_networks[f_index][s_index]->activate(state_network_input);
// 				test_state_vals[s_index] += this->test_state_networks[f_index][s_index]->output->acti_vals[0];
// 			}
// 		}
// 		flat_vals.erase(flat_vals.begin());

// 		vector<double> curr_score_network_input;
// 		for (int i_index = 0; i_index < this->curr_num_states; i_index++) {
// 			if (this->curr_state_not_needed_locally[f_index][i_index]) {
// 				curr_score_network_input.push_back(0.0);
// 			} else {
// 				curr_score_network_input.push_back(curr_state_vals[i_index]);
// 			}
// 		}
// 		this->curr_score_networks[f_index]->activate(curr_score_network_input);
// 		predicted_score += this->curr_score_networks[f_index]->output->acti_vals[0];

// 		vector<double> test_score_network_input;
// 		for (int i_index = 0; i_index < this->curr_num_states; i_index++) {
// 			if (this->test_state_not_needed_locally[f_index][i_index]) {
// 				test_score_network_input.push_back(0.0);
// 			} else {
// 				test_score_network_input.push_back(test_state_vals[i_index]);
// 			}
// 		}
// 		this->test_score_networks[f_index]->activate(test_score_network_input);

// 		for (int s_index = 0; s_index < this->curr_num_states_cleared[f_index]; s_index++) {
// 			curr_state_vals[s_index] = 0.0;
// 		}
// 		for (int s_index = 0; s_index < this->test_num_states_cleared[f_index]; s_index++) {
// 			test_state_vals[s_index] = 0.0;
// 		}
// 	}
// }

// void Fold::clean_backprop(double target_val,
// 						  double final_misguess,
// 						  double& predicted_score) {
// 	vector<double> curr_state_errors(this->curr_num_states, 0.0);
// 	vector<double> test_state_errors(this->curr_num_states, 0.0);

// 	for (int f_index = this->sequence_length-1; f_index >= 0; f_index--) {
// 		double test_score_network_error = this->curr_score_networks[f_index]->output->acti_vals[0]
// 			- this->test_score_networks[f_index]->output->acti_vals[0];
// 		this->sum_error += abs(test_score_network_error);
// 		vector<double> test_score_network_errors{test_score_network_error};
// 		if (this->state_iter <= 130000) {
// 			this->test_score_networks[f_index]->backprop(test_score_network_errors, 0.01);
// 		} else {
// 			this->test_score_networks[f_index]->backprop(test_score_network_errors, 0.002);
// 		}
// 		for (int i_index = 0; i_index < this->curr_num_states; i_index++) {
// 			if (!this->test_state_not_needed_locally[f_index][i_index]) {
// 				test_state_errors[i_index] += this->test_score_networks[f_index]->input->errors[i_index];
// 				this->test_score_networks[f_index]->input->errors[i_index] = 0.0;
// 			}
// 		}

// 		vector<double> curr_score_network_errors{target_val - predicted_score};
// 		this->curr_score_networks[f_index]->backprop(curr_score_network_errors, 0.002);
// 		for (int i_index = 0; i_index < this->curr_num_states; i_index++) {
// 			if (!this->curr_state_not_needed_locally[f_index][i_index]) {
// 				curr_state_errors[i_index] += this->curr_score_networks[f_index]->input->errors[i_index];
// 				this->curr_score_networks[f_index]->input->errors[i_index] = 0.0;
// 			}
// 		}

// 		predicted_score -= this->curr_score_networks[f_index]->output->acti_vals[0];

// 		for (int s_index = this->curr_num_states-1; s_index >= 0; s_index--) {
// 			if (!this->test_state_networks_not_needed[f_index][s_index]) {
// 				vector<double> state_network_errors{test_state_errors[s_index]};
// 				if (this->state_iter <= 130000) {
// 					this->test_state_networks[f_index][s_index]->backprop(state_network_errors, 0.01);
// 				} else {
// 					this->test_state_networks[f_index][s_index]->backprop(state_network_errors, 0.002);
// 				}
// 				for (int i_index = 0; i_index < s_index+1; i_index++) {
// 					if (!this->test_state_not_needed_locally[f_index][i_index]) {
// 						test_state_errors[i_index] += this->test_state_networks[f_index][s_index]->input->errors[1+i_index];
// 						this->test_state_networks[f_index][s_index]->input->errors[1+i_index] = 0.0;
// 					}
// 				}
// 			}
// 		}

// 		for (int s_index = this->curr_num_states-1; s_index >= 0; s_index--) {
// 			if (!this->curr_state_networks_not_needed[f_index][s_index]) {
// 				vector<double> state_network_errors{curr_state_errors[s_index]};
// 				this->curr_state_networks[f_index][s_index]->backprop(state_network_errors, 0.002);
// 				for (int i_index = 0; i_index < s_index+1; i_index++) {
// 					if (!this->curr_state_not_needed_locally[f_index][i_index]) {
// 						curr_state_errors[i_index] += this->curr_state_networks[f_index][s_index]->input->errors[1+i_index];
// 						this->curr_state_networks[f_index][s_index]->input->errors[1+i_index] = 0.0;
// 					}
// 				}
// 			}
// 		}
// 	}

// 	clean_increment();
// }

// void Fold::clean_increment() {
// 	this->state_iter++;

// 	if (this->state_iter == 150000) {
// 		bool step_success = this->sum_error/this->sequence_length/10000 < 0.01;
// 		if (step_success) {
// 			cout << "SUCCESS" << endl;

// 			for (int f_index = 0; f_index < this->sequence_length; f_index++) {
// 				for (int s_index = 0; s_index < this->curr_num_states; s_index++) {
// 					if (!this->curr_state_networks_not_needed[f_index][s_index]) {
// 						delete this->curr_state_networks[f_index][s_index];
// 					}
// 				}

// 				delete this->curr_score_networks[f_index];
// 			}

// 			this->curr_state_networks_not_needed = this->test_state_networks_not_needed;
// 			this->curr_state_not_needed_locally = this->test_state_not_needed_locally;
// 			this->curr_num_states_cleared = this->test_num_states_cleared;
// 			this->curr_state_networks = this->test_state_networks;
// 			this->curr_score_networks = this->test_score_networks;
// 		} else {
// 			cout << "FAILURE" << endl;

// 			for (int f_index = 0; f_index < this->sequence_length; f_index++) {
// 				for (int s_index = 0; s_index < this->curr_num_states; s_index++) {
// 					if (!this->test_state_networks_not_needed[f_index][s_index]) {
// 						delete this->test_state_networks[f_index][s_index];
// 					}
// 				}

// 				delete this->test_score_networks[f_index];
// 			}
// 		}

// 		this->test_state_networks_not_needed = this->curr_state_networks_not_needed;
// 		this->test_state_not_needed_locally = this->curr_state_not_needed_locally;
// 		this->test_num_states_cleared = this->curr_num_states_cleared;

// 		bool next_step_done = false;
// 		while (!next_step_done) {
// 			if (this->state == STATE_REMOVE_NETWORK) {
// 				if (this->clean_state_index == 0) {
// 					this->state = STATE_REMOVE_STATE;
// 					this->clean_state_index = this->curr_num_states-1;

// 					if (this->curr_state_networks_not_needed[this->clean_step_index][this->clean_state_index]) {
// 						cout << "STATE_REMOVE_STATE " << this->clean_step_index << " " << this->clean_state_index << endl;

// 						next_step_done = true;
// 						this->test_state_not_needed_locally[this->clean_step_index][this->clean_state_index] = true;
// 					}
// 				} else {
// 					this->clean_state_index--;

// 					cout << "STATE_REMOVE_NETWORK " << this->clean_step_index << " " << this->clean_state_index << endl;

// 					next_step_done = true;
// 					this->test_state_networks_not_needed[this->clean_step_index][this->clean_state_index] = true;
// 				}
// 			} else if (this->state == STATE_REMOVE_STATE) {
// 				if (this->clean_state_index == 0) {
// 					this->state = STATE_CLEAR_STATE;

// 					if (this->clean_step_index == 0) {
// 						// do nothing -- this->curr_num_states_cleared[0] = 0
// 					} else {
// 						// just a small optimization that doesn't impact results
// 						this->curr_num_states_cleared[this->clean_step_index] = this->curr_num_states_cleared[this->clean_step_index-1];
// 						for (int s_index = 0; s_index < this->curr_num_states_cleared[this->clean_step_index]; s_index++) {
// 							if (!this->curr_state_networks_not_needed[this->clean_step_index][s_index]) {
// 								this->curr_num_states_cleared[this->clean_step_index] = s_index;
// 								break;
// 							}
// 						}
// 					}

// 					// don't worry about clean_state_index for STATE_CLEAR_STATE

// 					if (this->curr_num_states_cleared[this->clean_step_index] == this->curr_num_states-1) {
// 						// let next section handle
// 					} else {
// 						cout << "STATE_CLEAR_STATE " << this->clean_step_index << " " << this->test_num_states_cleared[this->clean_step_index] << endl;

// 						next_step_done = true;
// 						this->test_num_states_cleared[this->clean_step_index]++;
// 					}
// 				} else {
// 					this->clean_state_index--;

// 					if (this->curr_state_networks_not_needed[this->clean_step_index][this->clean_state_index]) {
// 						cout << "STATE_REMOVE_STATE " << this->clean_step_index << " " << this->clean_state_index << endl;

// 						next_step_done = true;
// 						this->test_state_not_needed_locally[this->clean_step_index][this->clean_state_index] = true;
// 					}
// 				}
// 			} else {
// 				// this->state == STATE_CLEAR_STATE
// 				if (!step_success || this->curr_num_states_cleared[this->clean_step_index] == this->curr_num_states-1) {
// 					if (this->clean_step_index == this->sequence_length-1) {
// 						cout << "STATE_DONE" << endl;

// 						next_step_done = true;
// 						this->state = STATE_DONE;
// 					} else {
// 						this->state = STATE_REMOVE_NETWORK;

// 						this->clean_step_index++;
// 						this->clean_state_index = this->curr_num_states-1;

// 						cout << "STATE_REMOVE_NETWORK " << this->clean_step_index << " " << this->clean_state_index << endl;

// 						next_step_done = true;
// 						this->test_state_networks_not_needed[this->clean_step_index][this->clean_state_index] = true;
// 					}
// 				} else {
// 					// TODO: potentially speed up by checking both previous num_states_cleared and state_networks_not_needed
// 					cout << "STATE_CLEAR_STATE " << this->clean_step_index << " " << this->test_num_states_cleared[this->clean_step_index] << endl;

// 					next_step_done = true;
// 					this->test_num_states_cleared[this->clean_step_index]++;
// 				}
// 			}
// 		}

// 		if (this->state != STATE_DONE) {
// 			for (int f_index = 0; f_index < this->sequence_length; f_index++) {
// 				for (int s_index = 0; s_index < this->curr_num_states; s_index++) {
// 					if (!this->test_state_networks_not_needed[f_index][s_index]) {
// 						this->test_state_networks[f_index][s_index] = new Network(this->curr_state_networks[f_index][s_index]);
// 					} else {
// 						this->test_state_networks[f_index][s_index] = NULL;
// 					}
// 				}

// 				this->test_score_networks[f_index] = new Network(this->curr_score_networks[f_index]);
// 			}
// 		}

// 		this->state_iter = 0;
// 		this->sum_error = 0.0;
// 	} else {
// 		if (this->state_iter%10000 == 0) {
// 			cout << "this->state_iter: " << this->state_iter << endl;
// 			cout << "this->sum_error: " << this->sum_error << endl;
// 			cout << endl;
// 			this->sum_error = 0.0;
// 		}
// 	}
// }

FoldHistory::FoldHistory(Fold* fold) {
	this->fold = fold;

	int num_total_states = fold->sum_inner_inputs
		+ fold->curr_num_new_inner_states
		+ fold->num_local_states
		+ fold->num_input_states
		+ fold->curr_num_new_outer_states;
	this->state_network_histories = vector<vector<StateNetworkHistory*>>(
		fold->sequence_length, vector<StateNetworkHistory*>(num_total_states, NULL));
	this->inner_scope_histories = vector<ScopeHistory*>(fold->sequence_length, NULL);
	this->score_network_updates = vector<double>(fold->sequence_length);
	this->score_network_histories = vector<StateNetworkHistory*>(fold->sequence_length, NULL);
}
