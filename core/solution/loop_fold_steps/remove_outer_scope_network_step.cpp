#include "loop_fold.h"

#include <iostream>

#include "action_node.h"
#include "constants.h"
#include "globals.h"
#include "scope_node.h"

using namespace std;

void LoopFold::remove_outer_scope_network_outer_scope_activate_helper(
		vector<double>& new_outer_state_vals,
		ScopeHistory* scope_history,
		RunHelper& run_helper,
		LoopFoldHistory* history,
		vector<double>& test_new_outer_state_vals,
		vector<vector<StateNetworkHistory*>>& test_outer_state_network_histories) {
	int scope_id = scope_history->scope->id;

	map<int, vector<vector<StateNetwork*>>>::iterator it = this->curr_outer_state_networks.find(scope_id);

	map<int, vector<vector<StateNetwork*>>>::iterator test_it;
	if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE || run_helper.explore_phase == EXPLORE_PHASE_NONE) {
		test_it = this->test_outer_state_networks.find(scope_id);
	}

	for (int i_index = 0; i_index < (int)scope_history->node_histories.size(); i_index++) {
		for (int h_index = 0; h_index < (int)scope_history->node_histories[i_index].size(); h_index++) {
			if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_ACTION) {
				if (it != this->curr_outer_state_networks.end()) {
					int node_id = scope_history->node_histories[i_index][h_index]->scope_index;
					if (node_id < (int)it->second.size()
							&& it->second[node_id].size() > 0) {
						ActionNodeHistory* action_node_history = (ActionNodeHistory*)scope_history->node_histories[i_index][h_index];
						for (int s_index = 0; s_index < this->curr_num_new_outer_states; s_index++) {
							if (!this->curr_outer_state_networks_not_needed[scope_id][node_id][s_index]) {
								it->second[node_id][s_index]->new_external_activate(
									action_node_history->obs_snapshot,
									action_node_history->ending_state_snapshot,
									new_outer_state_vals);
								new_outer_state_vals[s_index] += it->second[node_id][s_index]->output->acti_vals[0];
							}
						}
						// Note: don't save history as don't backprop outer errors into their scopes
						//   - too complicated as those scopes/errors won't be initialized
						//   - won't lead to ideal results, but would just need to wait until fold completes
					}
				}

				if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE || run_helper.explore_phase == EXPLORE_PHASE_NONE) {
					if (test_it != this->test_outer_state_networks.end()) {
						int node_id = scope_history->node_histories[i_index][h_index]->scope_index;
						if (node_id < (int)test_it->second.size()
								&& test_it->second[node_id].size() > 0) {
							test_outer_state_network_histories.push_back(vector<StateNetworkHistory*>());
							ActionNodeHistory* action_node_history = (ActionNodeHistory*)scope_history->node_histories[i_index][h_index];
							for (int s_index = 0; s_index < this->curr_num_new_outer_states; s_index++) {
								if (!this->test_outer_state_networks_not_needed[scope_id][node_id][s_index]) {
									StateNetworkHistory* state_network_history = new StateNetworkHistory(test_it->second[node_id][s_index]);
									test_it->second[node_id][s_index]->new_external_activate(
										action_node_history->obs_snapshot,
										action_node_history->ending_state_snapshot,
										test_new_outer_state_vals,
										state_network_history);
									test_outer_state_network_histories.back().push_back(state_network_history);
									test_new_outer_state_vals[s_index] += test_it->second[node_id][s_index]->output->acti_vals[0];
								} else {
									test_outer_state_network_histories.back().push_back(NULL);
								}
							}
						}
					}
				}
			} else if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_INNER_SCOPE) {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];
				remove_outer_scope_network_outer_scope_activate_helper(
					new_outer_state_vals,
					scope_node_history->inner_scope_history,
					run_helper,
					history,
					test_new_outer_state_vals,
					test_outer_state_network_histories);
			}
		}
	}
}

void LoopFold::remove_outer_scope_network_inner_scope_activate_helper(
		vector<double>& new_state_vals,
		ScopeHistory* scope_history,
		RunHelper& run_helper,
		int iter_index,
		int step_index,
		LoopFoldHistory* history) {
	int scope_id = scope_history->scope->id;

	map<int, vector<vector<StateNetwork*>>>::iterator it = this->curr_inner_state_networks.find(scope_id);

	for (int i_index = 0; i_index < (int)scope_history->node_histories.size(); i_index++) {
		for (int h_index = 0; h_index < (int)scope_history->node_histories[i_index].size(); h_index++) {
			if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_ACTION) {
				if (it != this->curr_inner_state_networks.end()) {
					int node_id = scope_history->node_histories[i_index][h_index]->scope_index;
					if (node_id < (int)it->second.size()
							&& it->second[node_id].size() > 0) {
						ActionNodeHistory* action_node_history = (ActionNodeHistory*)scope_history->node_histories[i_index][h_index];
						if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN) {
							history->inner_state_network_histories[iter_index][step_index].push_back(vector<StateNetworkHistory*>());
							for (int s_index = 0; s_index < this->curr_num_new_inner_states; s_index++) {
								StateNetworkHistory* state_network_history = new StateNetworkHistory(it->second[node_id][s_index]);
								it->second[node_id][s_index]->new_external_activate(
									action_node_history->obs_snapshot,
									action_node_history->ending_state_snapshot,
									new_state_vals,
									state_network_history);
								history->inner_state_network_histories[iter_index][step_index].back().push_back(state_network_history);
								new_state_vals[s_index] += it->second[node_id][s_index]->output->acti_vals[0];
							}
						} else {
							for (int s_index = 0; s_index < this->curr_num_new_inner_states; s_index++) {
								it->second[node_id][s_index]->new_external_activate(
									action_node_history->obs_snapshot,
									action_node_history->ending_state_snapshot,
									new_state_vals);
								new_state_vals[s_index] += it->second[node_id][s_index]->output->acti_vals[0];
							}
						}
					}
				}
			} else if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_INNER_SCOPE) {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];
				remove_outer_scope_network_inner_scope_activate_helper(
					new_state_vals,
					scope_node_history->inner_scope_history,
					run_helper,
					iter_index,
					step_index,
					history);
			}
		}
	}
}

void LoopFold::remove_outer_scope_network_activate(
		Problem& problem,
		vector<double>& state_vals,
		vector<bool>& states_initialized,
		double& predicted_score,
		double& scale_factor,
		double& sum_impact,
		vector<ScopeHistory*>& context_histories,
		RunHelper& run_helper,
		LoopFoldHistory* history) {
	vector<double> new_outer_state_vals(this->curr_num_new_outer_states, 0.0);

	vector<double> test_new_outer_state_vals;
	vector<vector<StateNetworkHistory*>> test_outer_state_network_histories;
	if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE || run_helper.explore_phase == EXPLORE_PHASE_NONE) {
		test_new_outer_state_vals = vector<double>(this->curr_num_new_outer_states, 0.0);
	}

	ScopeHistory* scope_history = context_histories[context_histories.size() - this->scope_context.size()];
	remove_outer_scope_network_outer_scope_activate_helper(
		new_outer_state_vals,
		scope_history,
		run_helper,
		history,
		test_new_outer_state_vals,
		test_outer_state_network_histories);

	if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE || run_helper.explore_phase == EXPLORE_PHASE_NONE) {
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
		for (int n_index = (int)test_outer_state_network_histories.size()-1; n_index >= 0; n_index--) {
			for (int o_index = this->curr_num_new_outer_states-1; o_index >= 0; o_index--) {
				if (test_outer_state_network_histories[n_index][o_index] != NULL) {
					StateNetwork* state_network = test_outer_state_network_histories[n_index][o_index]->network;
					state_network->new_external_backprop(
						test_new_outer_state_errors[o_index],
						test_new_outer_state_errors,
						target_max_update,
						test_outer_state_network_histories[n_index][o_index]);
				}
			}
		}

		for (int n_index = 0; n_index < (int)test_outer_state_network_histories.size(); n_index++) {
			for (int o_index = 0; o_index < (int)test_outer_state_network_histories[n_index].size(); o_index++) {
				if (test_outer_state_network_histories[n_index][o_index] != NULL) {
					delete test_outer_state_network_histories[n_index][o_index];
				}
			}
		}

		this->state_iter++;
		this->sub_iter++;
		history->state_iter_snapshot = this->state_iter;
	}

	vector<double> new_inner_state_vals(this->sum_inner_inputs + this->curr_num_new_inner_states, 0.0);

	history->starting_state_network_histories = vector<StateNetworkHistory*>(this->sum_inner_inputs + this->curr_num_new_inner_states);
	for (int i_index = 0; i_index < this->sum_inner_inputs + this->curr_num_new_inner_states; i_index++) {
		if (i_index >= this->sum_inner_inputs
				|| this->curr_inner_inputs_needed[i_index]) {
			if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN) {
				StateNetworkHistory* state_network_history = new StateNetworkHistory(this->curr_starting_state_networks[i_index]);
				this->curr_starting_state_networks[i_index]->new_sequence_activate(
					new_inner_state_vals,
					state_vals,
					new_outer_state_vals,
					state_network_history);
				history->starting_state_network_histories[i_index] = state_network_history;
			} else {
				this->curr_starting_state_networks[i_index]->new_sequence_activate(
					new_inner_state_vals,
					state_vals,
					new_outer_state_vals);
			}
			new_inner_state_vals[i_index] += this->curr_starting_state_networks[i_index]->output->acti_vals[0];
		}
	}

	int iter_index = 0;
	while (true) {
		StateNetworkHistory* continue_score_network_history = new StateNetworkHistory(this->curr_continue_score_network);
		this->curr_continue_score_network->new_sequence_activate(
			new_inner_state_vals,
			state_vals,
			new_outer_state_vals,
			continue_score_network_history);

		StateNetworkHistory* continue_misguess_network_history = new StateNetworkHistory(this->curr_continue_misguess_network);
		this->curr_continue_misguess_network->new_sequence_activate(
			new_inner_state_vals,
			state_vals,
			new_outer_state_vals,
			continue_misguess_network_history);

		StateNetworkHistory* halt_score_network_history = new StateNetworkHistory(this->curr_halt_score_network);
		this->curr_halt_score_network->new_sequence_activate(
			new_inner_state_vals,
			state_vals,
			new_outer_state_vals,
			halt_score_network_history);

		StateNetworkHistory* halt_misguess_network_history = new StateNetworkHistory(this->curr_halt_misguess_network);
		this->curr_halt_misguess_network->new_sequence_activate(
			new_inner_state_vals,
			state_vals,
			new_outer_state_vals,
			halt_misguess_network_history);

		bool is_halt;
		if (iter_index > 7) {
			// cap number of iters for now
			is_halt = true;
		} else {
			double score_diff = scale_factor*this->curr_continue_score_network->output->acti_vals[0]
				- scale_factor*this->curr_halt_score_network->output->acti_vals[0];
			double score_standard_deviation = abs(scale_factor)*sqrt(this->curr_score_variance);
			// TODO: not sure how network gradient descent corresponds to sample size, but simply set to 2500 for now
			double score_diff_t_value = score_diff
				/ (score_standard_deviation / sqrt(2500));
			if (score_diff_t_value > 2.326) {
				is_halt = false;
			} else if (score_diff_t_value < -2.326) {
				is_halt = true;
			} else {
				double misguess_diff = this->curr_continue_misguess_network->output->acti_vals[0]
					- this->curr_halt_misguess_network->output->acti_vals[0];
				double misguess_standard_deviation = sqrt(this->curr_misguess_variance);
				double misguess_diff_t_value = misguess_diff
					/ (misguess_standard_deviation / sqrt(2500));
				if (misguess_diff_t_value < -2.326) {
					is_halt = false;
				} else if (misguess_diff_t_value > 2.326) {
					is_halt = true;
				} else {
					// continue if no strong signal either way
					is_halt = false;
				}
			}
		}

		if (is_halt) {
			history->halt_score_network_update = this->curr_halt_score_network->output->acti_vals[0];
			history->halt_score_network_history = halt_score_network_history;

			history->halt_misguess_val = this->curr_halt_misguess_network->output->acti_vals[0];
			history->halt_misguess_network_history = halt_misguess_network_history;

			delete continue_score_network_history;
			delete continue_misguess_network_history;

			predicted_score += scale_factor*this->curr_halt_score_network->output->acti_vals[0];

			history->num_loop_iters = iter_index;
			
			break;
		} else {
			history->continue_score_network_updates.push_back(this->curr_continue_score_network->output->acti_vals[0]);
			history->continue_score_network_histories.push_back(continue_score_network_history);

			history->continue_misguess_vals.push_back(this->curr_continue_misguess_network->output->acti_vals[0]);
			history->continue_misguess_network_histories.push_back(continue_misguess_network_history);

			delete halt_score_network_history;
			delete halt_misguess_network_history;

			predicted_score += scale_factor*this->curr_continue_score_network->output->acti_vals[0];

			int num_inner_networks = this->sum_inner_inputs
				+ this->curr_num_new_inner_states
				+ this->num_states;

			history->inner_scope_histories.push_back(vector<ScopeHistory*>(this->sequence_length, NULL));
			history->score_network_updates.push_back(vector<double>(this->sequence_length));
			history->score_network_histories.push_back(vector<StateNetworkHistory*>(this->sequence_length, NULL));

			if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN) {
				history->state_network_histories.push_back(vector<vector<StateNetworkHistory*>>(
					this->sequence_length, vector<StateNetworkHistory*>(num_inner_networks, NULL)));
				history->inner_state_network_histories.push_back(vector<vector<vector<StateNetworkHistory*>>>(this->sequence_length, vector<vector<StateNetworkHistory*>>()));
			}

			for (int f_index = 0; f_index < this->sequence_length; f_index++) {
				if (this->is_inner_scope[f_index]) {
					for (int i_index = 0; i_index < this->inner_input_start_indexes[f_index] + this->num_inner_inputs[f_index]; i_index++) {
						if (!this->curr_state_networks_not_needed[f_index][i_index]) {
							if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN) {
								StateNetworkHistory* state_network_history = new StateNetworkHistory(this->curr_state_networks[f_index][i_index]);
								this->curr_state_networks[f_index][i_index]->new_sequence_activate(
									new_inner_state_vals,
									state_vals,
									new_outer_state_vals,
									state_network_history);
								history->state_network_histories[iter_index][f_index][i_index] = state_network_history;
							} else {
								this->curr_state_networks[f_index][i_index]->new_sequence_activate(
									new_inner_state_vals,
									state_vals,
									new_outer_state_vals);
							}
							new_inner_state_vals[i_index] += this->curr_state_networks[f_index][i_index]->output->acti_vals[0];
						}
					}

					Scope* inner_scope = solution->scopes[this->existing_scope_ids[f_index]];
					int num_input_states_diff = inner_scope->num_states - this->num_inner_inputs[f_index];

					vector<double> inner_input_vals(new_inner_state_vals.begin() + this->inner_input_start_indexes[f_index],
						new_inner_state_vals.begin() + this->inner_input_start_indexes[f_index] + this->num_inner_inputs[f_index]);
					inner_input_vals.insert(inner_input_vals.end(), num_input_states_diff, 0.0);

					vector<bool> inner_inputs_initialized(this->curr_inner_inputs_needed.begin() + this->inner_input_start_indexes[f_index],
						this->curr_inner_inputs_needed.begin() + this->inner_input_start_indexes[f_index] + this->num_inner_inputs[f_index]);
					inner_inputs_initialized.insert(inner_inputs_initialized.end(), num_input_states_diff, false);

					// unused
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
										  sum_impact,	// track impact in inner to simplify backprop
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
					// allow explore in inner for simplicity
					history->inner_scope_histories[iter_index][f_index] = scope_history;

					for (int i_index = 0; i_index < this->num_inner_inputs[f_index]; i_index++) {
						if (this->curr_inner_inputs_needed[this->inner_input_start_indexes[f_index] + i_index]) {
							new_inner_state_vals[this->inner_input_start_indexes[f_index] + i_index] = inner_input_vals[i_index];
						}
					}

					vector<double> new_state_vals;
					for (int i_index = 0; i_index < this->curr_num_new_inner_states; i_index++) {
						new_state_vals.push_back(new_inner_state_vals[this->sum_inner_inputs+i_index]);
					}
					remove_outer_scope_network_inner_scope_activate_helper(
						new_state_vals,
						scope_history,
						run_helper,
						iter_index,
						f_index,
						history);
					for (int i_index = 0; i_index < this->curr_num_new_inner_states; i_index++) {
						new_inner_state_vals[this->sum_inner_inputs+i_index] = new_state_vals[i_index];
					}

					for (int i_index = this->inner_input_start_indexes[f_index] + this->num_inner_inputs[f_index];
							i_index < this->sum_inner_inputs + this->curr_num_new_inner_states; i_index++) {
						if (!this->curr_state_networks_not_needed[f_index][i_index]) {
							if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN) {
								StateNetworkHistory* state_network_history = new StateNetworkHistory(this->curr_state_networks[f_index][i_index]);
								this->curr_state_networks[f_index][i_index]->new_sequence_activate(
									new_inner_state_vals,
									state_vals,
									new_outer_state_vals,
									state_network_history);
								history->state_network_histories[iter_index][f_index][i_index] = state_network_history;
							} else {
								this->curr_state_networks[f_index][i_index]->new_sequence_activate(
									new_inner_state_vals,
									state_vals,
									new_outer_state_vals);
							}
							new_inner_state_vals[i_index] += this->curr_state_networks[f_index][i_index]->output->acti_vals[0];
						}
					}
					for (int s_index = 0; s_index < this->num_states; s_index++) {
						if (states_initialized[s_index]) {
							int state_index = this->sum_inner_inputs
								+ this->curr_num_new_inner_states
								+ s_index;
							if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN) {
								StateNetworkHistory* state_network_history = new StateNetworkHistory(this->curr_state_networks[f_index][state_index]);
								this->curr_state_networks[f_index][state_index]->new_sequence_activate(
									new_inner_state_vals,
									state_vals,
									new_outer_state_vals,
									state_network_history);
								history->state_network_histories[iter_index][f_index][state_index] = state_network_history;
							} else {
								this->curr_state_networks[f_index][state_index]->new_sequence_activate(
									new_inner_state_vals,
									state_vals,
									new_outer_state_vals);
							}
							state_vals[s_index] += this->curr_state_networks[f_index][state_index]->output->acti_vals[0];
						}
					}
				} else {
					problem.perform_action(this->actions[f_index]);
					double obs = problem.get_observation();

					for (int i_index = 0; i_index < this->sum_inner_inputs + this->curr_num_new_inner_states; i_index++) {
						if (!this->curr_state_networks_not_needed[f_index][i_index]) {
							if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN) {
								StateNetworkHistory* state_network_history = new StateNetworkHistory(this->curr_state_networks[f_index][i_index]);
								this->curr_state_networks[f_index][i_index]->new_sequence_activate(
									obs,
									new_inner_state_vals,
									state_vals,
									new_outer_state_vals,
									state_network_history);
								history->state_network_histories[iter_index][f_index][i_index] = state_network_history;
							} else {
								this->curr_state_networks[f_index][i_index]->new_sequence_activate(
									obs,
									new_inner_state_vals,
									state_vals,
									new_outer_state_vals);
							}
							new_inner_state_vals[i_index] += this->curr_state_networks[f_index][i_index]->output->acti_vals[0];
						}
					}
					for (int s_index = 0; s_index < this->num_states; s_index++) {
						if (states_initialized[s_index]) {
							int state_index = this->sum_inner_inputs
								+ this->curr_num_new_inner_states
								+ s_index;
							if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN) {
								StateNetworkHistory* state_network_history = new StateNetworkHistory(this->curr_state_networks[f_index][state_index]);
								this->curr_state_networks[f_index][state_index]->new_sequence_activate(
									obs,
									new_inner_state_vals,
									state_vals,
									new_outer_state_vals,
									state_network_history);
								history->state_network_histories[iter_index][f_index][state_index] = state_network_history;
							} else {
								this->curr_state_networks[f_index][state_index]->new_sequence_activate(
									obs,
									new_inner_state_vals,
									state_vals,
									new_outer_state_vals);
							}
							state_vals[s_index] += this->curr_state_networks[f_index][state_index]->output->acti_vals[0];
						}
					}
				}

				StateNetworkHistory* score_network_history = new StateNetworkHistory(this->curr_score_networks[f_index]);
				this->curr_score_networks[f_index]->new_sequence_activate(
					new_inner_state_vals,
					state_vals,
					new_outer_state_vals,
					score_network_history);
				history->score_network_histories[iter_index][f_index] = score_network_history;
				history->score_network_updates[iter_index][f_index] = this->curr_score_networks[f_index]->output->acti_vals[0];
				predicted_score += scale_factor*this->curr_score_networks[f_index]->output->acti_vals[0];
			}
		}

		iter_index++;
	}
}

// LOOP_FOLD_STATE_REMOVE_OUTER_NETWORK backprop generic
