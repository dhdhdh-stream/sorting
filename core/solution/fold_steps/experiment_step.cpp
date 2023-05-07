#include "fold.h"

#include <cmath>
#include <iostream>

#include "action_node.h"
#include "constants.h"
#include "globals.h"
#include "scope_node.h"

using namespace std;

void Fold::experiment_outer_scope_activate_helper(vector<double>& new_outer_state_vals,
												  ScopeHistory* scope_history,
												  FoldHistory* history) {
	int scope_id = scope_history->scope->id;

	map<int, vector<vector<StateNetwork*>>>::iterator it = this->test_outer_state_networks.find(scope_id);
	if (it == this->test_outer_state_networks.end()) {
		it = this->test_outer_state_networks.insert({scope_id, vector<vector<StateNetwork*>>()}).first;
	}

	int size_diff = (int)scope_history->scope->nodes.size() - (int)it->second.size();
	it->second.insert(it->second.end(), size_diff, vector<StateNetwork*>());

	for (int i_index = 0; i_index < (int)scope_history->node_histories.size(); i_index++) {
		for (int h_index = 0; h_index < (int)scope_history->node_histories[i_index].size(); h_index++) {
			if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_ACTION) {
				int node_id = scope_history->node_histories[i_index][h_index]->scope_index;
				if (it->second[node_id].size() == 0) {
					for (int s_index = 0; s_index < this->test_num_new_outer_states; s_index++) {
						it->second[node_id].push_back(new StateNetwork(
							1,
							scope_history->scope->num_states,
							0,
							this->test_num_new_outer_states,
							20));
					}
				}
				history->outer_state_network_histories.push_back(vector<StateNetworkHistory*>());
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)scope_history->node_histories[i_index][h_index];
				for (int s_index = 0; s_index < this->test_num_new_outer_states; s_index++) {
					StateNetworkHistory* state_network_history = new StateNetworkHistory(it->second[node_id][s_index]);
					it->second[node_id][s_index]->new_external_activate(
						action_node_history->obs_snapshot,
						action_node_history->ending_state_snapshot,
						new_outer_state_vals,
						state_network_history);
					history->outer_state_network_histories.back().push_back(state_network_history);
					new_outer_state_vals[s_index] += it->second[node_id][s_index]->output->acti_vals[0];
				}
			} else if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_INNER_SCOPE) {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];
				experiment_outer_scope_activate_helper(new_outer_state_vals,
													   scope_node_history->inner_scope_history,
													   history);
			}
		}
	}
}

void Fold::experiment_score_activate(vector<double>& state_vals,
									 double& predicted_score,
									 double& scale_factor,
									 vector<ScopeHistory*>& context_histories,
									 RunHelper& run_helper,
									 FoldHistory* history) {
	run_helper.explore_phase = EXPLORE_PHASE_EXPERIMENT_LEARN;

	vector<double> new_outer_state_vals(this->test_num_new_outer_states, 0.0);

	ScopeHistory* scope_history = context_histories[context_histories.size() - this->scope_context.size()];
	experiment_outer_scope_activate_helper(new_outer_state_vals,
										   scope_history,
										   history);

	StateNetworkHistory* starting_score_network_history = new StateNetworkHistory(this->test_starting_score_network);
	this->test_starting_score_network->new_external_activate(
		state_vals,
		new_outer_state_vals,
		starting_score_network_history);
	history->starting_score_update = this->test_starting_score_network->output->acti_vals[0];
	history->starting_score_network_history = starting_score_network_history;

	history->new_outer_state_vals = new_outer_state_vals;

	// modify predicted_score in score_activate for experiment
	predicted_score += scale_factor*this->test_starting_score_network->output->acti_vals[0];
}

void Fold::experiment_inner_scope_activate_helper(vector<double>& new_state_vals,	// only num_new_inner_states, no sum_inner_inputs
												  ScopeHistory* scope_history,
												  int step_index,
												  FoldHistory* history) {
	int scope_id = scope_history->scope->id;

	map<int, vector<vector<StateNetwork*>>>::iterator it = this->test_inner_state_networks.find(scope_id);
	if (it == this->test_inner_state_networks.end()) {
		it = this->test_inner_state_networks.insert({scope_id, vector<vector<StateNetwork*>>()}).first;
	}

	int size_diff = (int)scope_history->scope->nodes.size() - (int)it->second.size();
	it->second.insert(it->second.end(), size_diff, vector<StateNetwork*>());

	for (int i_index = 0; i_index < (int)scope_history->node_histories.size(); i_index++) {
		for (int h_index = 0; h_index < (int)scope_history->node_histories[i_index].size(); h_index++) {
			if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_ACTION) {
				int node_id = scope_history->node_histories[i_index][h_index]->scope_index;
				if (it->second[node_id].size() == 0) {
					for (int s_index = 0; s_index < this->test_num_new_inner_states; s_index++) {
						it->second[node_id].push_back(new StateNetwork(
							1,
							scope_history->scope->num_states,
							0,
							this->test_num_new_inner_states,
							20));
					}
				}
				history->inner_state_network_histories[step_index].push_back(vector<StateNetworkHistory*>());
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)scope_history->node_histories[i_index][h_index];
				for (int s_index = 0; s_index < this->test_num_new_inner_states; s_index++) {
					StateNetworkHistory* state_network_history = new StateNetworkHistory(it->second[node_id][s_index]);
					it->second[node_id][s_index]->new_external_activate(
						action_node_history->obs_snapshot,
						action_node_history->ending_state_snapshot,
						new_state_vals,
						state_network_history);
					history->inner_state_network_histories[step_index].back().push_back(state_network_history);
					new_state_vals[s_index] += it->second[node_id][s_index]->output->acti_vals[0];
				}
			} else if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_INNER_SCOPE) {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];
				experiment_inner_scope_activate_helper(new_state_vals,
													   scope_node_history->inner_scope_history,
													   step_index,
													   history);
			}
		}
	}
}

void Fold::experiment_sequence_activate(Problem& problem,
										vector<double>& state_vals,
										vector<bool>& states_initialized,
										double& predicted_score,
										double& scale_factor,
										RunHelper& run_helper,
										FoldHistory* history) {
	run_helper.flat_scope_id = this->scope_context[this->exit_depth];

	vector<double> new_inner_state_vals(this->sum_inner_inputs + this->test_num_new_inner_states, 0.0);
	vector<double> new_outer_state_vals = history->new_outer_state_vals;

	int num_inner_networks = this->sum_inner_inputs
		+ this->test_num_new_inner_states
		+ this->num_sequence_states;
	history->state_network_histories = vector<vector<StateNetworkHistory*>>(
		this->sequence_length, vector<StateNetworkHistory*>(num_inner_networks, NULL));
	history->inner_scope_histories = vector<ScopeHistory*>(this->sequence_length, NULL);
	history->score_network_updates = vector<double>(this->sequence_length);
	history->score_network_histories = vector<StateNetworkHistory*>(this->sequence_length, NULL);
	history->inner_state_network_histories = vector<vector<vector<StateNetworkHistory*>>>(this->sequence_length, vector<vector<StateNetworkHistory*>>());

	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		if (this->is_inner_scope[f_index]) {
			for (int i_index = 0; i_index < this->inner_input_start_indexes[f_index] + this->num_inner_inputs[f_index]; i_index++) {
				StateNetworkHistory* state_network_history = new StateNetworkHistory(this->test_state_networks[f_index][i_index]);
				// don't check obs for is_inner_scope
				this->test_state_networks[f_index][i_index]->new_sequence_activate(
					new_inner_state_vals,
					state_vals,
					new_outer_state_vals,
					state_network_history);
				history->state_network_histories[f_index][i_index] = state_network_history;

				new_inner_state_vals[i_index] += this->test_state_networks[f_index][i_index]->output->acti_vals[0];
			}

			scale_factor *= this->inner_scope_scale_mods[f_index]->weight;

			Scope* inner_scope = solution->scopes[this->existing_scope_ids[f_index]];
			int num_input_states_diff = inner_scope->num_states - this->num_inner_inputs[f_index];

			vector<double> inner_input_vals(new_inner_state_vals.begin() + this->inner_input_start_indexes[f_index],
				new_inner_state_vals.begin() + this->inner_input_start_indexes[f_index] + this->num_inner_inputs[f_index]);
			inner_input_vals.insert(inner_input_vals.end(), num_input_states_diff, 0.0);

			vector<bool> inner_inputs_initialized(this->num_inner_inputs[f_index], true);
			inner_inputs_initialized.insert(inner_inputs_initialized.end(), num_input_states_diff, false);

			// unused
			double inner_sum_impact = 0.0;
			vector<int> inner_scope_context;
			vector<int> inner_node_context;
			vector<ScopeHistory*> inner_context_histories;
			int inner_early_exit_depth;
			int inner_early_exit_node_id;
			FoldHistory* inner_early_exit_fold_history;
			int inner_explore_exit_depth;
			int inner_explore_exit_node_id;
			FoldHistory* inner_explore_exit_fold_history;

			ScopeHistory* scope_history = new ScopeHistory(inner_scope);
			inner_scope->activate(problem,
								  inner_input_vals,
								  inner_inputs_initialized,
								  predicted_score,
								  scale_factor,
								  inner_sum_impact,
								  inner_scope_context,
								  inner_node_context,
								  inner_context_histories,
								  inner_early_exit_depth,
								  inner_early_exit_node_id,
								  inner_early_exit_fold_history,
								  inner_explore_exit_depth,
								  inner_explore_exit_node_id,
								  inner_explore_exit_fold_history,
								  run_helper,
								  scope_history);
			history->inner_scope_histories[f_index] = scope_history;

			for (int i_index = 0; i_index < this->num_inner_inputs[f_index]; i_index++) {
				new_inner_state_vals[this->inner_input_start_indexes[f_index] + i_index] = inner_input_vals[i_index];
			}

			vector<double> new_state_vals(new_inner_state_vals.begin()+this->sum_inner_inputs,
				new_inner_state_vals.begin()+this->sum_inner_inputs+this->test_num_new_inner_states);
			experiment_inner_scope_activate_helper(new_state_vals,
												   scope_history,
												   f_index,
												   history);
			for (int i_index = 0; i_index < this->test_num_new_inner_states; i_index++) {
				new_inner_state_vals[this->sum_inner_inputs+i_index] = new_state_vals[i_index];
			}

			scale_factor /= this->inner_scope_scale_mods[f_index]->weight;

			// update back state so have chance to compress front after
			for (int i_index = this->inner_input_start_indexes[f_index] + this->num_inner_inputs[f_index];
					i_index < this->sum_inner_inputs + this->test_num_new_inner_states; i_index++) {
				StateNetworkHistory* state_network_history = new StateNetworkHistory(this->test_state_networks[f_index][i_index]);
				this->test_state_networks[f_index][i_index]->new_sequence_activate(
					new_inner_state_vals,
					state_vals,
					new_outer_state_vals,
					state_network_history);
				history->state_network_histories[f_index][i_index] = state_network_history;

				new_inner_state_vals[i_index] += this->test_state_networks[f_index][i_index]->output->acti_vals[0];
			}
			for (int s_index = 0; s_index < this->num_sequence_states; s_index++) {
				if (states_initialized[s_index]) {
					int state_index = this->sum_inner_inputs
						+ this->test_num_new_inner_states
						+ s_index;
					StateNetworkHistory* state_network_history = new StateNetworkHistory(this->test_state_networks[f_index][state_index]);
					this->test_state_networks[f_index][state_index]->new_sequence_activate(
						new_inner_state_vals,
						state_vals,
						new_outer_state_vals,
						state_network_history);
					history->state_network_histories[f_index][state_index] = state_network_history;

					state_vals[s_index] += this->test_state_networks[f_index][state_index]->output->acti_vals[0];
				}
			}
		} else {
			problem.perform_action(this->actions[f_index]);
			double obs = problem.get_observation();

			for (int i_index = 0; i_index < this->sum_inner_inputs + this->test_num_new_inner_states; i_index++) {
				StateNetworkHistory* state_network_history = new StateNetworkHistory(this->test_state_networks[f_index][i_index]);
				this->test_state_networks[f_index][i_index]->new_sequence_activate(
					obs,
					new_inner_state_vals,
					state_vals,
					new_outer_state_vals,
					state_network_history);
				history->state_network_histories[f_index][i_index] = state_network_history;

				new_inner_state_vals[i_index] += this->test_state_networks[f_index][i_index]->output->acti_vals[0];
			}
			for (int s_index = 0; s_index < this->num_sequence_states; s_index++) {
				if (states_initialized[s_index]) {
					int state_index = this->sum_inner_inputs
						+ this->test_num_new_inner_states
						+ s_index;
					StateNetworkHistory* state_network_history = new StateNetworkHistory(this->test_state_networks[f_index][state_index]);
					this->test_state_networks[f_index][state_index]->new_sequence_activate(
						obs,
						new_inner_state_vals,
						state_vals,
						new_outer_state_vals,
						state_network_history);
					history->state_network_histories[f_index][state_index] = state_network_history;

					state_vals[s_index] += this->test_state_networks[f_index][state_index]->output->acti_vals[0];
				}
			}
		}

		StateNetworkHistory* score_network_history = new StateNetworkHistory(this->test_score_networks[f_index]);
		this->test_score_networks[f_index]->new_sequence_activate(
			new_inner_state_vals,
			state_vals,
			new_outer_state_vals,
			score_network_history);
		history->score_network_histories[f_index] = score_network_history;
		history->score_network_updates[f_index] = this->test_score_networks[f_index]->output->acti_vals[0];
		predicted_score += scale_factor*this->test_score_networks[f_index]->output->acti_vals[0];
	}

	predicted_score += this->end_mod;

	if (run_helper.is_recursive) {
		this->is_recursive++;
	}
}

void Fold::experiment_backprop(vector<double>& state_errors,
							   vector<bool>& states_initialized,
							   double target_val,
							   double final_diff,
							   double final_misguess,
							   double& predicted_score,
							   double& scale_factor,
							   RunHelper& run_helper,
							   FoldHistory* history) {
	this->end_mod = 0.9999*this->end_mod + 0.0001*final_diff;
	predicted_score -= this->end_mod;

	this->test_replace_average_score = 0.9999*this->test_replace_average_score + 0.0001*target_val;
	this->test_replace_average_misguess = 0.9999*this->test_replace_average_misguess + 0.0001*final_misguess;
	double curr_misguess_variance = (this->test_replace_average_misguess - final_misguess)*(this->test_replace_average_misguess - final_misguess);
	this->test_replace_misguess_variance = 0.9999*this->test_replace_misguess_variance + 0.0001*curr_misguess_variance;

	this->sum_error += abs(target_val-predicted_score);

	vector<double> new_inner_state_errors(this->sum_inner_inputs+this->test_num_new_inner_states, 0.0);
	vector<double> new_outer_state_errors(this->test_num_new_outer_states, 0.0);

	for (int f_index = this->sequence_length-1; f_index >= 0; f_index--) {
		double predicted_score_error = target_val - predicted_score;
		double score_network_target_max_update;
		if (this->state == FOLD_STATE_EXPERIMENT && this->state_iter <= 100000) {
			score_network_target_max_update = 0.05;
		} else if (this->state_iter <= 400000) {
			score_network_target_max_update = 0.01;
		} else {
			score_network_target_max_update = 0.002;
		}
		this->test_score_networks[f_index]->new_sequence_backprop(
			scale_factor*predicted_score_error,
			new_inner_state_errors,
			state_errors,
			new_outer_state_errors,
			score_network_target_max_update,
			history->score_network_histories[f_index]);

		predicted_score -= scale_factor*history->score_network_updates[f_index];

		if (this->is_inner_scope[f_index]) {
			for (int s_index = this->num_sequence_states-1; s_index >= 0; s_index--) {
				if (states_initialized[s_index]) {
					int state_index = this->sum_inner_inputs
						+ this->test_num_new_inner_states
						+ s_index;
					double state_network_target_max_update;
					if (this->state == FOLD_STATE_EXPERIMENT && this->state_iter <= 100000) {
						state_network_target_max_update = 0.05;
					} else if (this->state_iter <= 400000) {
						state_network_target_max_update = 0.01;
					} else {
						state_network_target_max_update = 0.002;
					}
					this->test_state_networks[f_index][state_index]->new_sequence_backprop(
						state_errors[s_index],
						new_inner_state_errors,
						state_errors,
						new_outer_state_errors,
						state_network_target_max_update,
						history->state_network_histories[f_index][state_index]);
				}
			}
			for (int i_index = this->sum_inner_inputs+this->test_num_new_inner_states-1;
					i_index >= this->inner_input_start_indexes[f_index] + this->num_inner_inputs[f_index]; i_index--) {
				double state_network_target_max_update;
				if ((this->state == FOLD_STATE_EXPERIMENT || (this->state == FOLD_STATE_ADD_INNER_STATE && i_index == this->sum_inner_inputs+this->test_num_new_inner_states-1))
						&& this->state_iter <= 100000) {
					state_network_target_max_update = 0.05;
				} else if (this->state_iter <= 400000) {
					state_network_target_max_update = 0.01;
				} else {
					state_network_target_max_update = 0.002;
				}
				this->test_state_networks[f_index][i_index]->new_sequence_backprop(
					new_inner_state_errors[i_index],
					new_inner_state_errors,
					state_errors,
					new_outer_state_errors,
					state_network_target_max_update,
					history->state_network_histories[f_index][i_index]);
			}

			scale_factor *= this->inner_scope_scale_mods[f_index]->weight;

			vector<double> new_state_errors(new_inner_state_errors.begin()+this->sum_inner_inputs,
				new_inner_state_errors.begin()+this->sum_inner_inputs+this->test_num_new_inner_states);
			for (int n_index = (int)history->inner_state_network_histories[f_index].size()-1; n_index >= 0; n_index--) {
				for (int i_index = this->test_num_new_inner_states-1; i_index >= 0; i_index--) {
					StateNetwork* state_network = history->inner_state_network_histories[f_index][n_index][i_index]->network;
					double state_network_target_max_update;
					if ((this->state == FOLD_STATE_EXPERIMENT || (this->state == FOLD_STATE_ADD_INNER_STATE && i_index == this->test_num_new_inner_states-1))
							&& this->state_iter <= 100000) {
						state_network_target_max_update = 0.05;
					} else if (this->state_iter <= 400000) {
						state_network_target_max_update = 0.01;
					} else {
						state_network_target_max_update = 0.002;
					}
					state_network->new_external_backprop(
						new_state_errors[i_index],
						new_state_errors,
						state_network_target_max_update,
						history->inner_state_network_histories[f_index][n_index][i_index]);
				}
			}
			for (int i_index = 0; i_index < this->test_num_new_inner_states; i_index++) {
				new_inner_state_errors[this->sum_inner_inputs+i_index] = new_state_errors[i_index];
			}

			Scope* inner_scope = solution->scopes[this->existing_scope_ids[f_index]];
			int num_input_states_diff = inner_scope->num_states - this->num_inner_inputs[f_index];

			vector<double> inner_input_errors(new_inner_state_errors.begin() + this->inner_input_start_indexes[f_index],
				new_inner_state_errors.begin() + this->inner_input_start_indexes[f_index] + this->num_inner_inputs[f_index]);
			inner_input_errors.insert(inner_input_errors.end(), num_input_states_diff, 0.0);

			vector<bool> inner_inputs_initialized(this->num_inner_inputs[f_index], true);
			inner_inputs_initialized.insert(inner_inputs_initialized.end(), num_input_states_diff, false);

			// unused
			double inner_final_sum_impact = 0.0;

			double scope_scale_factor_error = 0.0;
			inner_scope->backprop(inner_input_errors,
								  inner_inputs_initialized,
								  target_val,
								  final_diff,
								  final_misguess,
								  inner_final_sum_impact,
								  predicted_score,
								  scale_factor,
								  scope_scale_factor_error,
								  run_helper,
								  history->inner_scope_histories[f_index]);

			for (int i_index = 0; i_index < this->num_inner_inputs[f_index]; i_index++) {
				new_inner_state_errors[this->inner_input_start_indexes[f_index] + i_index] = inner_input_errors[i_index];
			}

			scale_factor /= this->inner_scope_scale_mods[f_index]->weight;

			this->inner_scope_scale_mods[f_index]->backprop(scope_scale_factor_error, 0.0002);

			for (int i_index = this->inner_input_start_indexes[f_index]+this->num_inner_inputs[f_index]-1; i_index >= 0; i_index--) {
				double state_network_target_max_update;
				if (this->state == FOLD_STATE_EXPERIMENT && this->state_iter <= 100000) {
					state_network_target_max_update = 0.05;
				} else if (this->state_iter <= 400000) {
					state_network_target_max_update = 0.01;
				} else {
					state_network_target_max_update = 0.002;
				}
				this->test_state_networks[f_index][i_index]->new_sequence_backprop(
					new_inner_state_errors[i_index],
					new_inner_state_errors,
					state_errors,
					new_outer_state_errors,
					state_network_target_max_update,
					history->state_network_histories[f_index][i_index]);
			}
		} else {
			for (int s_index = this->num_sequence_states-1; s_index >= 0; s_index--) {
				if (states_initialized[s_index]) {
					int state_index = this->sum_inner_inputs
						+ this->test_num_new_inner_states
						+ s_index;
					double state_network_target_max_update;
					if (this->state == FOLD_STATE_EXPERIMENT && this->state_iter <= 100000) {
						state_network_target_max_update = 0.05;
					} else if (this->state_iter <= 400000) {
						state_network_target_max_update = 0.01;
					} else {
						state_network_target_max_update = 0.002;
					}
					this->test_state_networks[f_index][state_index]->new_sequence_backprop(
						state_errors[s_index],
						new_inner_state_errors,
						state_errors,
						new_outer_state_errors,
						state_network_target_max_update,
						history->state_network_histories[f_index][state_index]);
				}
			}
			for (int i_index = this->sum_inner_inputs+this->test_num_new_inner_states-1; i_index >= 0; i_index--) {
				double state_network_target_max_update;
				if (this->state == FOLD_STATE_EXPERIMENT && this->state_iter <= 100000) {
					state_network_target_max_update = 0.05;
				} else if (this->state_iter <= 400000) {
					state_network_target_max_update = 0.01;
				} else {
					state_network_target_max_update = 0.002;
				}
				this->test_state_networks[f_index][i_index]->new_sequence_backprop(
					new_inner_state_errors[i_index],
					new_inner_state_errors,
					state_errors,
					new_outer_state_errors,
					state_network_target_max_update,
					history->state_network_histories[f_index][i_index]);
			}
		}
	}

	double starting_score_network_target_max_update;
	if (this->state == FOLD_STATE_EXPERIMENT && this->state_iter <= 100000) {
		starting_score_network_target_max_update = 0.05;
	} else if (this->state_iter <= 400000) {
		starting_score_network_target_max_update = 0.01;
	} else {
		starting_score_network_target_max_update = 0.002;
	}
	this->test_starting_score_network->new_external_backprop(
		target_val - predicted_score,
		new_outer_state_errors,
		starting_score_network_target_max_update,
		history->starting_score_network_history);

	// no need to update predicted_score as end of backprop

	if (scale_factor*history->starting_score_update > 0.0) {
		this->test_branch_average_score = 0.9999*this->test_branch_average_score + 0.0001*target_val;
	} else {
		this->test_branch_average_score = 0.9999*this->test_branch_average_score + 0.0001*predicted_score;

		double existing_improvement = predicted_score - target_val;
		this->test_existing_average_improvement = 0.9999*this->test_existing_average_improvement + 0.0001*existing_improvement;
	}

	for (int n_index = (int)history->outer_state_network_histories.size()-1; n_index >= 0; n_index--) {
		for (int o_index = this->test_num_new_outer_states-1; o_index >= 0; o_index--) {
			StateNetwork* state_network = history->outer_state_network_histories[n_index][o_index]->network;
			double state_network_target_max_update;
			if ((this->state == FOLD_STATE_EXPERIMENT || (this->state == FOLD_STATE_ADD_OUTER_STATE && o_index == this->test_num_new_outer_states-1))
					&& this->state_iter <= 100000) {
				state_network_target_max_update = 0.05;
			} else if (this->state_iter <= 400000) {
				state_network_target_max_update = 0.01;
			} else {
				state_network_target_max_update = 0.002;
			}
			state_network->new_external_backprop(
				new_outer_state_errors[o_index],
				new_outer_state_errors,
				state_network_target_max_update,
				history->outer_state_network_histories[n_index][o_index]);
		}
	}

	experiment_increment();
}
