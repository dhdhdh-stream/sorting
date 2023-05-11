#include "loop_fold.h"

#include <iostream>

#include "action_node.h"
#include "constants.h"
#include "globals.h"
#include "scope_node.h"

using namespace std;

void LoopFold::experiment_learn_outer_scope_activate_helper(
		vector<double>& new_outer_state_vals,
		ScopeHistory* scope_history,
		LoopFoldHistory* history) {
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
				experiment_learn_outer_scope_activate_helper(
					new_outer_state_vals,
					scope_node_history->inner_scope_history,
					history);
			}
		}
	}
}

void LoopFold::experiment_learn_inner_scope_activate_helper(
		vector<double>& new_state_vals,
		ScopeHistory* scope_history,
		int iter_index,
		int step_index,
		LoopFoldHistory* history) {
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
				history->inner_state_network_histories[iter_index][step_index].push_back(vector<StateNetworkHistory*>());
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)scope_history->node_histories[i_index][h_index];
				for (int s_index = 0; s_index < this->test_num_new_inner_states; s_index++) {
					StateNetworkHistory* state_network_history = new StateNetworkHistory(it->second[node_id][s_index]);
					it->second[node_id][s_index]->new_external_activate(
						action_node_history->obs_snapshot,
						action_node_history->ending_state_snapshot,
						new_state_vals,
						state_network_history);
					history->inner_state_network_histories[iter_index][step_index].back().push_back(state_network_history);
					new_state_vals[s_index] += it->second[node_id][s_index]->output->acti_vals[0];
				}
			} else if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_INNER_SCOPE) {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];
				experiment_learn_inner_scope_activate_helper(
					new_state_vals,
					scope_node_history->inner_scope_history,
					iter_index,
					step_index,
					history);
			}
		}
	}
}

void LoopFold::experiment_learn_activate(Problem& problem,
										 vector<double>& state_vals,
										 vector<bool>& states_initialized,
										 double& predicted_score,
										 double& scale_factor,
										 vector<ScopeHistory*>& context_histories,
										 RunHelper& run_helper,
										 LoopFoldHistory* history) {
	run_helper.explore_phase = EXPLORE_PHASE_EXPERIMENT_LEARN;

	vector<double> new_outer_state_vals(this->test_num_new_outer_states, 0.0);

	ScopeHistory* scope_history = context_histories[context_histories.size() - this->scope_context.size()];
	experiment_learn_outer_scope_activate_helper(
		new_outer_state_vals,
		scope_history,
		history);

	vector<double> new_inner_state_vals(this->sum_inner_inputs + this->test_num_new_inner_states, 0.0);

	history->starting_state_network_histories = vector<StateNetworkHistory*>(this->sum_inner_inputs + this->test_num_new_inner_states);
	for (int i_index = 0; i_index < this->sum_inner_inputs + this->test_num_new_inner_states; i_index++) {
		StateNetworkHistory* state_network_history = new StateNetworkHistory(this->test_starting_state_networks[i_index]);
		this->test_starting_state_networks[i_index]->new_sequence_activate(
			new_inner_state_vals,
			state_vals,
			new_outer_state_vals,
			state_network_history);
		history->starting_state_network_histories[i_index] = state_network_history;

		new_inner_state_vals[i_index] += this->test_starting_state_networks[i_index]->output->acti_vals[0];
	}

	int loop_iters = rand()%7;
	history->num_loop_iters = loop_iters;

	for (int iter_index = 0; iter_index < loop_iters; iter_index++) {
		this->test_halt_score_network->new_sequence_activate(
			new_inner_state_vals,
			state_vals,
			new_outer_state_vals);
		history->halt_score_snapshots.push_back(predicted_score + scale_factor*this->test_halt_score_network->output->acti_vals[0]);

		StateNetworkHistory* continue_score_network_history = new StateNetworkHistory(this->test_continue_score_network);
		this->test_continue_score_network->new_sequence_activate(
			new_inner_state_vals,
			state_vals,
			new_outer_state_vals,
			continue_score_network_history);
		history->continue_score_network_histories.push_back(continue_score_network_history);
		history->continue_score_snapshots.push_back(predicted_score + scale_factor*this->test_continue_score_network->output->acti_vals[0]);

		this->test_halt_misguess_network->new_sequence_activate(
			new_inner_state_vals,
			state_vals,
			new_outer_state_vals);
		history->halt_misguess_snapshots.push_back(abs(scale_factor)*this->test_halt_misguess_network->output->acti_vals[0]);

		StateNetworkHistory* continue_misguess_network_history = new StateNetworkHistory(this->test_continue_misguess_network);
		this->test_continue_misguess_network->new_sequence_activate(
			new_inner_state_vals,
			state_vals,
			new_outer_state_vals,
			continue_misguess_network_history);
		history->continue_misguess_network_histories.push_back(continue_misguess_network_history);
		history->continue_misguess_snapshots.push_back(abs(scale_factor)*this->test_continue_misguess_network->output->acti_vals[0]);

		int num_inner_networks = this->sum_inner_inputs
			+ this->test_num_new_inner_states
			+ this->num_states;
		history->state_network_histories.push_back(vector<vector<StateNetworkHistory*>>(
			this->sequence_length, vector<StateNetworkHistory*>(num_inner_networks, NULL)));
		history->inner_scope_histories.push_back(vector<ScopeHistory*>(this->sequence_length, NULL));
		history->score_network_updates.push_back(vector<double>(this->sequence_length));
		history->score_network_histories.push_back(vector<StateNetworkHistory*>(this->sequence_length, NULL));
		history->inner_state_network_histories.push_back(vector<vector<vector<StateNetworkHistory*>>>(this->sequence_length, vector<vector<StateNetworkHistory*>>()));

		for (int f_index = 0; f_index < this->sequence_length; f_index++) {
			if (this->is_inner_scope[f_index]) {
				for (int i_index = 0; i_index < this->inner_input_start_indexes[f_index] + this->num_inner_inputs[f_index]; i_index++) {
					StateNetworkHistory* state_network_history = new StateNetworkHistory(this->test_state_networks[f_index][i_index]);
					this->test_state_networks[f_index][i_index]->new_sequence_activate(
						new_inner_state_vals,
						state_vals,
						new_outer_state_vals,
						state_network_history);
					history->state_network_histories[iter_index][f_index][i_index] = state_network_history;

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
				history->inner_scope_histories[iter_index][f_index] = scope_history;

				for (int i_index = 0; i_index < this->num_inner_inputs[f_index]; i_index++) {
					new_inner_state_vals[this->inner_input_start_indexes[f_index] + i_index] = inner_input_vals[i_index];
				}

				vector<double> new_state_vals;
				for (int i_index = 0; i_index < this->test_num_new_inner_states; i_index++) {
					new_state_vals.push_back(new_inner_state_vals[this->sum_inner_inputs+i_index]);
				}
				experiment_learn_inner_scope_activate_helper(
					new_state_vals,
					scope_history,
					iter_index,
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
					history->state_network_histories[iter_index][f_index][i_index] = state_network_history;

					new_inner_state_vals[i_index] += this->test_state_networks[f_index][i_index]->output->acti_vals[0];
				}
				for (int s_index = 0; s_index < this->num_states; s_index++) {
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
						history->state_network_histories[iter_index][f_index][state_index] = state_network_history;

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
					history->state_network_histories[iter_index][f_index][i_index] = state_network_history;

					new_inner_state_vals[i_index] += this->test_state_networks[f_index][i_index]->output->acti_vals[0];
				}
				for (int s_index = 0; s_index < this->num_states; s_index++) {
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
						history->state_network_histories[iter_index][f_index][state_index] = state_network_history;

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
			history->score_network_updates[iter_index][f_index] = this->test_score_networks[f_index]->output->acti_vals[0];
			history->score_network_histories[iter_index][f_index] = score_network_history;
			predicted_score += scale_factor*this->test_score_networks[f_index]->output->acti_vals[0];
		}
	}

	StateNetworkHistory* halt_score_network_history = new StateNetworkHistory(this->test_halt_score_network);
	this->test_halt_score_network->new_sequence_activate(
		new_inner_state_vals,
		state_vals,
		new_outer_state_vals,
		halt_score_network_history);
	history->halt_score_network_history = halt_score_network_history;
	history->halt_score_snapshots.push_back(predicted_score + scale_factor*this->test_halt_score_network->output->acti_vals[0]);

	StateNetworkHistory* halt_misguess_network_history = new StateNetworkHistory(this->test_halt_misguess_network);
	this->test_halt_misguess_network->new_sequence_activate(
		new_inner_state_vals,
		state_vals,
		new_outer_state_vals,
		halt_misguess_network_history);
	history->halt_misguess_network_history = halt_misguess_network_history;
	history->halt_misguess_snapshots.push_back(scale_factor*this->test_halt_misguess_network->output->acti_vals[0]);

	predicted_score += this->end_mod;
}

void LoopFold::experiment_learn_backprop(vector<double>& state_errors,
										 vector<bool>& states_initialized,
										 double target_val,
										 double final_diff,
										 double final_misguess,
										 double& predicted_score,
										 double& scale_factor,
										 RunHelper& run_helper,
										 LoopFoldHistory* history) {
	this->test_average_score = 0.9999*this->test_average_score + 0.0001*target_val;
	double score_variance = (this->test_average_score - target_val)*(this->test_average_score - target_val);
	this->test_score_variance = 0.9999*this->test_score_variance + 0.0001*score_variance;
	this->test_average_misguess = 0.9999*this->test_average_misguess + 0.0001*final_misguess;
	double misguess_variance = (this->test_average_misguess - final_misguess)*(this->test_average_misguess - final_misguess);
	this->test_misguess_variance = 0.9999*this->test_misguess_variance + 0.0001*misguess_variance;

	this->sum_error += abs(target_val - predicted_score);

	this->end_mod = 0.9999*this->end_mod + 0.0001*final_diff;
	predicted_score -= this->end_mod;

	vector<double> new_inner_state_errors(this->sum_inner_inputs+this->test_num_new_inner_states, 0.0);
	vector<double> new_outer_state_errors(this->test_num_new_outer_states, 0.0);

	double halt_predicted_score_error = target_val - history->halt_score_snapshots.back();
	double halt_score_network_target_max_update;
	if (this->state == LOOP_FOLD_STATE_EXPERIMENT && this->state_iter <= 100000) {
		halt_score_network_target_max_update = 0.05;
	} else if (this->state_iter <= 400000) {
		halt_score_network_target_max_update = 0.01;
	} else {
		halt_score_network_target_max_update = 0.002;
	}
	this->test_halt_score_network->new_sequence_backprop(
		scale_factor*halt_predicted_score_error,
		new_inner_state_errors,
		state_errors,
		new_outer_state_errors,
		halt_score_network_target_max_update,
		history->halt_score_network_history);

	double halt_misguess_error = abs(halt_predicted_score_error) - history->halt_misguess_snapshots.back();
	double halt_misguess_network_target_max_update;
	if (this->state == LOOP_FOLD_STATE_EXPERIMENT && this->state_iter <= 100000) {
		halt_misguess_network_target_max_update = 0.05;
	} else if (this->state_iter <= 400000) {
		halt_misguess_network_target_max_update = 0.01;
	} else {
		halt_misguess_network_target_max_update = 0.002;
	}
	this->test_halt_misguess_network->new_sequence_backprop(
		abs(scale_factor)*halt_misguess_error,
		new_inner_state_errors,
		state_errors,
		new_outer_state_errors,
		halt_misguess_network_target_max_update,
		history->halt_misguess_network_history);

	for (int iter_index = history->num_loop_iters-1; iter_index >= 0; iter_index--) {
		for (int f_index = this->sequence_length-1; f_index >= 0; f_index--) {
			double predicted_score_error = target_val - predicted_score;
			double score_network_target_max_update;
			if (this->state == LOOP_FOLD_STATE_EXPERIMENT && this->state_iter <= 100000) {
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
				history->score_network_histories[iter_index][f_index]);

			predicted_score -= scale_factor*history->score_network_updates[iter_index][f_index];

			if (this->is_inner_scope[f_index]) {
				for (int s_index = this->num_states-1; s_index >= 0; s_index--) {
					if (states_initialized[s_index]) {
						int state_index = this->sum_inner_inputs
							+ this->test_num_new_inner_states
							+ s_index;
						double state_network_target_max_update;
						if (this->state == LOOP_FOLD_STATE_EXPERIMENT && this->state_iter <= 100000) {
							state_network_target_max_update = 0.01;
						} else if (this->state_iter <= 400000) {
							state_network_target_max_update = 0.002;
						} else {
							state_network_target_max_update = 0.0005;
						}
						this->test_state_networks[f_index][state_index]->new_sequence_backprop(
							state_errors[s_index],
							new_inner_state_errors,
							state_errors,
							new_outer_state_errors,
							state_network_target_max_update,
							history->state_network_histories[iter_index][f_index][state_index]);
					}
				}
				for (int i_index = this->sum_inner_inputs+this->test_num_new_inner_states-1;
						i_index >= this->inner_input_start_indexes[f_index] + this->num_inner_inputs[f_index]; i_index--) {
					double state_network_target_max_update;
					if ((this->state == LOOP_FOLD_STATE_EXPERIMENT || (this->state == LOOP_FOLD_STATE_ADD_INNER_STATE && i_index == this->sum_inner_inputs+this->test_num_new_inner_states-1))
							&& this->state_iter <= 100000) {
						state_network_target_max_update = 0.01;
					} else if (this->state_iter <= 400000) {
						state_network_target_max_update = 0.002;
					} else {
						state_network_target_max_update = 0.0005;
					}
					this->test_state_networks[f_index][i_index]->new_sequence_backprop(
						new_inner_state_errors[i_index],
						new_inner_state_errors,
						state_errors,
						new_outer_state_errors,
						state_network_target_max_update,
						history->state_network_histories[iter_index][f_index][i_index]);
				}

				scale_factor *= this->inner_scope_scale_mods[f_index]->weight;

				vector<double> new_state_errors(new_inner_state_errors.begin()+this->sum_inner_inputs,
					new_inner_state_errors.begin()+this->sum_inner_inputs+this->test_num_new_inner_states);
				for (int n_index = (int)history->inner_state_network_histories[iter_index][f_index].size()-1; n_index >= 0; n_index--) {
					for (int i_index = this->test_num_new_inner_states-1; i_index >= 0; i_index--) {
						StateNetwork* state_network = history->inner_state_network_histories[iter_index][f_index][n_index][i_index]->network;
						double state_network_target_max_update;
						if ((this->state == LOOP_FOLD_STATE_EXPERIMENT || (this->state == LOOP_FOLD_STATE_ADD_INNER_STATE && i_index == this->test_num_new_inner_states-1))
								&& this->state_iter <= 100000) {
							state_network_target_max_update = 0.01;
						} else if (this->state_iter <= 400000) {
							state_network_target_max_update = 0.002;
						} else {
							state_network_target_max_update = 0.0005;
						}
						state_network->new_external_backprop(
							new_state_errors[i_index],
							new_state_errors,
							state_network_target_max_update,
							history->inner_state_network_histories[iter_index][f_index][n_index][i_index]);
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
									  history->inner_scope_histories[iter_index][f_index]);

				for (int i_index = 0; i_index < this->num_inner_inputs[f_index]; i_index++) {
					new_inner_state_errors[this->inner_input_start_indexes[f_index] + i_index] = inner_input_errors[i_index];
				}

				scale_factor /= this->inner_scope_scale_mods[f_index]->weight;

				this->inner_scope_scale_mods[f_index]->backprop(scope_scale_factor_error, 0.0002);

				for (int i_index = this->inner_input_start_indexes[f_index]+this->num_inner_inputs[f_index]-1; i_index >= 0; i_index--) {
					double state_network_target_max_update;
					if (this->state == LOOP_FOLD_STATE_EXPERIMENT && this->state_iter <= 100000) {
						state_network_target_max_update = 0.01;
					} else if (this->state_iter <= 400000) {
						state_network_target_max_update = 0.002;
					} else {
						state_network_target_max_update = 0.0005;
					}
					this->test_state_networks[f_index][i_index]->new_sequence_backprop(
						new_inner_state_errors[i_index],
						new_inner_state_errors,
						state_errors,
						new_outer_state_errors,
						state_network_target_max_update,
						history->state_network_histories[iter_index][f_index][i_index]);
				}
			} else {
				for (int s_index = this->num_states-1; s_index >= 0; s_index--) {
					if (states_initialized[s_index]) {
						int state_index = this->sum_inner_inputs
							+ this->test_num_new_inner_states
							+ s_index;
						double state_network_target_max_update;
						if (this->state == LOOP_FOLD_STATE_EXPERIMENT && this->state_iter <= 100000) {
							state_network_target_max_update = 0.01;
						} else if (this->state_iter <= 400000) {
							state_network_target_max_update = 0.002;
						} else {
							state_network_target_max_update = 0.0005;
						}
						this->test_state_networks[f_index][state_index]->new_sequence_backprop(
							state_errors[s_index],
							new_inner_state_errors,
							state_errors,
							new_outer_state_errors,
							state_network_target_max_update,
							history->state_network_histories[iter_index][f_index][state_index]);
					}
				}
				for (int i_index = this->sum_inner_inputs+this->test_num_new_inner_states-1; i_index >= 0; i_index--) {
					double state_network_target_max_update;
					if ((this->state == LOOP_FOLD_STATE_EXPERIMENT || (this->state == LOOP_FOLD_STATE_ADD_INNER_STATE && i_index == this->sum_inner_inputs+this->test_num_new_inner_states-1))
							&& this->state_iter <= 100000) {
						state_network_target_max_update = 0.01;
					} else if (this->state_iter <= 400000) {
						state_network_target_max_update = 0.002;
					} else {
						state_network_target_max_update = 0.0005;
					}
					this->test_state_networks[f_index][i_index]->new_sequence_backprop(
						new_inner_state_errors[i_index],
						new_inner_state_errors,
						state_errors,
						new_outer_state_errors,
						state_network_target_max_update,
						history->state_network_histories[iter_index][f_index][i_index]);
				}
			}
		}

		double best_halt_score = history->halt_score_snapshots.back();
		double best_halt_misguess = abs(target_val - history->halt_score_snapshots.back());
		for (int ii_index = history->num_loop_iters-1;	// back to front
				ii_index >= iter_index + 1; ii_index--) {
			double score_diff = history->halt_score_snapshots[ii_index] - best_halt_score;
			// fold variance not representative yet, so use existing variance for now
			double score_standard_deviation = sqrt(*this->existing_score_variance);
			// not a t-test, just looking for meaningful difference
			double score_val = score_diff / abs(scale_factor) / score_standard_deviation;
			if (score_val > 0.1) {
				best_halt_score = history->halt_score_snapshots[ii_index];
				best_halt_misguess = abs(target_val - history->halt_score_snapshots[ii_index]);
			} else if (score_val < -0.1) {
				continue;
			} else {
				double misguess_diff = abs(target_val - history->halt_score_snapshots[ii_index]) - best_halt_misguess;
				// fold variance not representative yet, so use existing variance for now
				double misguess_standard_deviation = sqrt(*this->existing_misguess_variance);
				double misguess_val = misguess_diff / abs(scale_factor) / misguess_standard_deviation;
				if (misguess_val < -0.1) {
					best_halt_score = history->halt_score_snapshots[ii_index];
					best_halt_misguess = abs(target_val - history->halt_score_snapshots[ii_index]);
				} else if (misguess_val > 0.1) {
					continue;
				} else {
					// use earlier iter if no strong signal either way
					best_halt_score = history->halt_score_snapshots[ii_index];
					best_halt_misguess = abs(target_val - history->halt_score_snapshots[ii_index]);
				}
			}
		}

		double continue_predicted_score_error = best_halt_score - history->continue_score_snapshots[iter_index];
		double continue_score_network_target_max_update;
		if (this->state == LOOP_FOLD_STATE_EXPERIMENT && this->state_iter <= 100000) {
			continue_score_network_target_max_update = 0.05;
		} else if (this->state_iter <= 400000) {
			continue_score_network_target_max_update = 0.01;
		} else {
			continue_score_network_target_max_update = 0.002;
		}
		this->test_continue_score_network->new_sequence_backprop(
			scale_factor*continue_predicted_score_error,
			new_inner_state_errors,
			state_errors,
			new_outer_state_errors,
			continue_score_network_target_max_update,
			history->continue_score_network_histories[iter_index]);

		double continue_misguess_error = best_halt_misguess - history->continue_misguess_snapshots[iter_index];
		double continue_misguess_network_target_max_update;
		if (this->state == LOOP_FOLD_STATE_EXPERIMENT && this->state_iter <= 100000) {
			continue_misguess_network_target_max_update = 0.05;
		} else if (this->state_iter <= 400000) {
			continue_misguess_network_target_max_update = 0.01;
		} else {
			continue_misguess_network_target_max_update = 0.002;
		}
		this->test_continue_misguess_network->new_sequence_backprop(
			abs(scale_factor)*continue_misguess_error,
			new_inner_state_errors,
			state_errors,
			new_outer_state_errors,
			continue_misguess_network_target_max_update,
			history->continue_misguess_network_histories[iter_index]);
	}

	for (int i_index = this->sum_inner_inputs+this->test_num_new_inner_states-1; i_index >= 0; i_index--) {
		double state_network_target_max_update;
		if ((this->state == LOOP_FOLD_STATE_EXPERIMENT || (this->state == LOOP_FOLD_STATE_ADD_INNER_STATE && i_index == this->sum_inner_inputs+this->test_num_new_inner_states-1))
				&& this->state_iter <= 100000) {
			state_network_target_max_update = 0.05;
		} else if (this->state_iter <= 400000) {
			state_network_target_max_update = 0.01;
		} else {
			state_network_target_max_update = 0.002;
		}
		this->test_starting_state_networks[i_index]->new_sequence_backprop(
			new_inner_state_errors[i_index],
			new_inner_state_errors,
			state_errors,
			new_outer_state_errors,
			state_network_target_max_update,
			history->starting_state_network_histories[i_index]);
	}

	for (int n_index = (int)history->outer_state_network_histories.size()-1; n_index >= 0; n_index--) {
		for (int o_index = this->test_num_new_outer_states-1; o_index >= 0; o_index--) {
			StateNetwork* state_network = history->outer_state_network_histories[n_index][o_index]->network;
			double state_network_target_max_update;
			if ((this->state == LOOP_FOLD_STATE_EXPERIMENT || (this->state == LOOP_FOLD_STATE_ADD_OUTER_STATE && o_index == this->test_num_new_outer_states-1))
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
