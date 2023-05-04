#include "loop_fold.h"

#include <iostream>

#include "action_node.h"
#include "constants.h"
#include "globals.h"
#include "scope_node.h"

using namespace std;

void LoopFold::clean_outer_scope_activate_helper(vector<double>& new_outer_state_vals,
												 ScopeHistory* scope_history,
												 vector<int>& curr_scope_context,
												 vector<int>& curr_node_context,
												 RunHelper& run_helper,
												 LoopFoldHistory* history) {
	int scope_id = scope_history->scope->id;
	curr_scope_context.push_back(scope_id);
	curr_node_context.push_back(-1);

	map<int, vector<vector<StateNetwork*>>>::iterator it = this->curr_outer_state_networks.find(scope_id);

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
			} else if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_INNER_SCOPE) {
				curr_node_context.back() = scope_history->node_histories[i_index][h_index]->scope_index;

				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];
				clean_outer_scope_activate_helper(new_outer_state_vals,
												  scope_node_history->inner_scope_history,
												  curr_scope_context,
												  curr_node_context,
												  run_helper,
												  history);

				curr_node_context.back() = -1;
			}
		}
	}

	curr_scope_context.pop_back();
	curr_node_context.pop_back();
}

void LoopFold::clean_inner_scope_activate_helper(vector<double>& new_state_vals,
												 ScopeHistory* scope_history,
												 vector<int>& curr_scope_context,
												 vector<int>& curr_node_context,
												 RunHelper& run_helper,
												 int iter_index,
												 int step_index,
												 LoopFoldHistory* history,
												 vector<double>& test_new_state_vals,
												 vector<vector<vector<vector<StateNetworkHistory*>>>>& test_inner_state_network_histories) {
	int scope_id = scope_history->scope->id;
	curr_scope_context.push_back(scope_id);
	curr_node_context.push_back(-1);

	map<int, vector<vector<StateNetwork*>>>::iterator it = this->curr_inner_state_networks.find(scope_id);

	map<int, vector<vector<StateNetwork*>>>::iterator test_it;
	if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE || run_helper.explore_phase == EXPLORE_PHASE_NONE) {
		test_it = this->test_inner_state_networks.find(scope_id);

		if (this->state == LOOP_FOLD_STATE_REMOVE_INNER_SCOPE) {
			set<int>::iterator needed_it = this->reverse_test_inner_scopes_needed.find(scope_id);
			if (needed_it != this->reverse_test_inner_scopes_needed.end()) {
				for (int c_index = 0; c_index < (int)curr_scope_context.size()-1; c_index++) {
					this->reverse_test_inner_scopes_needed.insert(curr_scope_context[c_index]);
					this->reverse_test_inner_contexts_needed.insert(make_pair(curr_scope_context[c_index], curr_node_context[c_index]));
				}
			}
		}
	}

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
								if (this->state == LOOP_FOLD_STATE_REMOVE_INNER_SCOPE
										|| !this->curr_inner_state_networks_not_needed[scope_id][node_id][s_index]) {
									StateNetworkHistory* state_network_history = new StateNetworkHistory(it->second[node_id][s_index]);
									it->second[node_id][s_index]->new_external_activate(
										action_node_history->obs_snapshot,
										action_node_history->ending_state_snapshot,
										new_state_vals,
										state_network_history);
									history->inner_state_network_histories[iter_index][step_index].back().push_back(state_network_history);
									new_state_vals[s_index] += it->second[node_id][s_index]->output->acti_vals[0];
								} else {
									history->inner_state_network_histories[iter_index][step_index].back().push_back(NULL);
								}
							}
						} else {
							for (int s_index = 0; s_index < this->curr_num_new_inner_states; s_index++) {
								if (this->state == LOOP_FOLD_STATE_REMOVE_INNER_SCOPE
										|| !this->curr_inner_state_networks_not_needed[scope_id][node_id][s_index]) {
									it->second[node_id][s_index]->new_external_activate(
										action_node_history->obs_snapshot,
										action_node_history->ending_state_snapshot,
										new_state_vals);
									new_state_vals[s_index] += it->second[node_id][s_index]->output->acti_vals[0];
								}
							}
						}
					}
				}

				if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE || run_helper.explore_phase == EXPLORE_PHASE_NONE) {
					if (test_it != this->test_inner_state_networks.end()) {
						int node_id = scope_history->node_histories[i_index][h_index]->scope_index;
						if (node_id < (int)test_it->second.size()
								&& test_it->second[node_id].size() > 0) {
							test_inner_state_network_histories[iter_index][step_index].push_back(vector<StateNetworkHistory*>());
							ActionNodeHistory* action_node_history = (ActionNodeHistory*)scope_history->node_histories[i_index][h_index];
							for (int s_index = 0; s_index < this->curr_num_new_inner_states; s_index++) {
								bool is_needed = false;
								if (this->state == LOOP_FOLD_STATE_REMOVE_INNER_SCOPE) {
									is_needed = true;
								} else {
									if (this->state == LOOP_FOLD_STATE_REMOVE_INNER_SCOPE_NETWORK) {
										is_needed = !this->test_inner_state_networks_not_needed[scope_id][node_id][s_index];
									} else {
										is_needed = !this->curr_inner_state_networks_not_needed[scope_id][node_id][s_index];
									}
								}
								if (is_needed) {
									StateNetworkHistory* state_network_history = new StateNetworkHistory(test_it->second[node_id][s_index]);
									test_it->second[node_id][s_index]->new_external_activate(
										action_node_history->obs_snapshot,
										action_node_history->ending_state_snapshot,
										test_new_state_vals,
										state_network_history);
									test_inner_state_network_histories[iter_index][step_index].back().push_back(state_network_history);
									test_new_state_vals[s_index] += test_it->second[node_id][s_index]->output->acti_vals[0];
								} else {
									test_inner_state_network_histories[iter_index][step_index].back().push_back(NULL);
								}
							}
						}
					}
				}
			} else if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_INNER_SCOPE) {
				curr_node_context.back() = scope_history->node_histories[i_index][h_index]->scope_index;

				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];
				clean_inner_scope_activate_helper(new_state_vals,
												  scope_node_history->inner_scope_history,
												  curr_scope_context,
												  curr_node_context,
												  run_helper,
												  iter_index,
												  step_index,
												  history,
												  test_new_state_vals,
												  test_inner_state_network_histories);

				curr_node_context.back() = -1;
			}
		}
	}

	curr_scope_context.pop_back();
	curr_node_context.pop_back();
}

void LoopFold::clean_activate(Problem& problem,
							  vector<double>& state_vals,
							  vector<bool>& states_initialized,
							  double& predicted_score,
							  double& scale_factor,
							  double& sum_impact,
							  vector<ScopeHistory*>& context_histories,
							  RunHelper& run_helper,
							  LoopFoldHistory* history) {
	vector<double> new_outer_state_vals(this->curr_num_new_outer_states, 0.0);

	ScopeHistory* scope_history = context_histories[context_histories.size() - this->scope_context.size()];
	vector<int> curr_scope_context;
	vector<int> curr_node_context;
	clean_outer_scope_activate_helper(new_outer_state_vals,
									  scope_history,
									  curr_scope_context,
									  curr_node_context,
									  run_helper,
									  history);

	vector<double> new_inner_state_vals(this->sum_inner_inputs + this->curr_num_new_inner_states, 0.0);

	vector<double> test_new_inner_state_vals;
	vector<double> test_state_vals;
	vector<double> test_new_outer_state_vals;
	if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE || run_helper.explore_phase == EXPLORE_PHASE_NONE) {
		test_new_inner_state_vals = vector<double>(this->sum_inner_inputs + this->curr_num_new_inner_states, 0.0);
		test_state_vals = state_vals;
		test_new_outer_state_vals = new_outer_state_vals;
	}

	history->starting_state_network_histories = vector<StateNetworkHistory*>(this->sum_inner_inputs + this->curr_num_new_inner_states, NULL);
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

	vector<StateNetworkHistory*> test_starting_state_network_histories(this->sum_inner_inputs + this->curr_num_new_inner_states, NULL);
	if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE || run_helper.explore_phase == EXPLORE_PHASE_NONE) {
		for (int i_index = 0; i_index < this->sum_inner_inputs + this->curr_num_new_inner_states; i_index++) {
			if (i_index >= this->sum_inner_inputs
					|| this->curr_inner_inputs_needed[i_index]) {
				StateNetworkHistory* state_network_history = new StateNetworkHistory(this->test_starting_state_networks[i_index]);
				this->test_starting_state_networks[i_index]->new_sequence_activate(
					test_new_inner_state_vals,
					test_state_vals,
					test_new_outer_state_vals,
					state_network_history);
				test_starting_state_network_histories[i_index] = state_network_history;

				test_new_inner_state_vals[i_index] += this->test_starting_state_networks[i_index]->output->acti_vals[0];
			}
		}
	}

	vector<vector<vector<double>>> inner_input_vals_snapshots;
	vector<vector<vector<double>>> test_inner_input_vals_snapshots;

	vector<StateNetworkHistory*> test_continue_score_network_histories;
	vector<double> test_continue_score_diff;
	vector<StateNetworkHistory*> test_continue_misguess_network_histories;
	vector<double> test_continue_misguess_diff;
	vector<StateNetworkHistory*> test_halt_score_network_histories;
	vector<double> test_halt_score_diff;
	vector<StateNetworkHistory*> test_halt_misguess_network_histories;
	vector<double> test_halt_misguess_diff;

	vector<vector<vector<StateNetworkHistory*>>> test_state_network_histories;
	vector<vector<ScopeHistory*>> test_inner_scope_histories;
	vector<vector<double>> test_score_network_updates;
	vector<vector<StateNetworkHistory*>> test_score_network_histories;
	vector<vector<vector<vector<StateNetworkHistory*>>>> test_inner_state_network_histories;

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

		// Note: test histories will have an extra iteration for ending
		if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE || run_helper.explore_phase == EXPLORE_PHASE_NONE) {
			StateNetworkHistory* test_continue_score_network_history = new StateNetworkHistory(this->test_continue_score_network);
			this->test_continue_score_network->new_sequence_activate(
				test_new_inner_state_vals,
				test_state_vals,
				test_new_outer_state_vals,
				test_continue_score_network_history);
			test_continue_score_network_histories.push_back(test_continue_score_network_history);
			test_continue_score_diff.push_back(this->curr_continue_score_network->output->acti_vals[0]
				- this->test_continue_score_network->output->acti_vals[0]);

			StateNetworkHistory* test_continue_misguess_network_history = new StateNetworkHistory(this->test_continue_misguess_network);
			this->test_continue_misguess_network->new_sequence_activate(
				test_new_inner_state_vals,
				test_state_vals,
				test_new_outer_state_vals,
				test_continue_misguess_network_history);
			test_continue_misguess_network_histories.push_back(test_continue_misguess_network_history);
			test_continue_misguess_diff.push_back(this->curr_continue_misguess_network->output->acti_vals[0]
				- this->test_continue_misguess_network->output->acti_vals[0]);

			StateNetworkHistory* test_halt_score_network_history = new StateNetworkHistory(this->test_halt_score_network);
			this->test_halt_score_network->new_sequence_activate(
				test_new_inner_state_vals,
				test_state_vals,
				test_new_outer_state_vals,
				test_halt_score_network_history);
			test_halt_score_network_histories.push_back(test_halt_score_network_history);
			test_halt_score_diff.push_back(this->curr_halt_score_network->output->acti_vals[0]
				- this->test_halt_score_network->output->acti_vals[0]);

			StateNetworkHistory* test_halt_misguess_network_history = new StateNetworkHistory(this->test_halt_misguess_network);
			this->test_halt_misguess_network->new_sequence_activate(
				test_new_inner_state_vals,
				test_state_vals,
				test_new_outer_state_vals,
				test_halt_misguess_network_history);
			test_halt_misguess_network_histories.push_back(test_halt_misguess_network_history);
			test_halt_misguess_diff.push_back(this->curr_halt_misguess_network->output->acti_vals[0]
				- this->test_halt_misguess_network->output->acti_vals[0]);
		}

		bool is_halt;
		is_halt = rand()%3 == 0;
		// if (iter_index > 7) {
		// 	// cap number of iters for now
		// 	is_halt = true;
		// } else {
		// 	double score_diff = scale_factor*this->curr_continue_score_network->output->acti_vals[0]
		// 		- scale_factor*this->curr_halt_score_network->output->acti_vals[0];
		// 	double score_standard_deviation = abs(scale_factor)*sqrt(this->curr_score_variance);
		// 	// TODO: not sure how network gradient descent corresponds to sample size, but simply set to 2500 for now
		// 	double score_diff_t_value = score_diff
		// 		/ (score_standard_deviation / sqrt(2500));
		// 	if (score_diff_t_value > 2.326) {
		// 		is_halt = false;
		// 	} else if (score_diff_t_value < -2.326) {
		// 		is_halt = true;
		// 	} else {
		// 		double misguess_diff = this->curr_continue_misguess_network->output->acti_vals[0]
		// 			- this->curr_halt_misguess_network->output->acti_vals[0];
		// 		double misguess_standard_deviation = sqrt(this->curr_misguess_variance);
		// 		double misguess_diff_t_value = misguess_diff
		// 			/ (misguess_standard_deviation / sqrt(2500));
		// 		if (misguess_diff_t_value < -2.326) {
		// 			is_halt = false;
		// 		} else if (misguess_diff_t_value > 2.326) {
		// 			is_halt = true;
		// 		} else {
		// 			// continue if no strong signal either way
		// 			is_halt = false;
		// 		}
		// 	}
		// }

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

			if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE || run_helper.explore_phase == EXPLORE_PHASE_NONE) {
				inner_input_vals_snapshots.push_back(vector<vector<double>>(this->sequence_length));
				test_inner_input_vals_snapshots.push_back(vector<vector<double>>(this->sequence_length));

				test_state_network_histories.push_back(vector<vector<StateNetworkHistory*>>(
					this->sequence_length, vector<StateNetworkHistory*>(num_inner_networks, NULL)));
				test_inner_scope_histories.push_back(vector<ScopeHistory*>(this->sequence_length, NULL));
				test_score_network_updates.push_back(vector<double>(this->sequence_length));
				test_score_network_histories.push_back(vector<StateNetworkHistory*>(this->sequence_length, NULL));
				test_inner_state_network_histories.push_back(vector<vector<vector<StateNetworkHistory*>>>(this->sequence_length, vector<vector<StateNetworkHistory*>>()));
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

						if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE || run_helper.explore_phase == EXPLORE_PHASE_NONE) {
							if (!this->test_state_networks_not_needed[f_index][i_index]) {
								StateNetworkHistory* state_network_history = new StateNetworkHistory(this->test_state_networks[f_index][i_index]);
								this->test_state_networks[f_index][i_index]->new_sequence_activate(
									test_new_inner_state_vals,
									test_state_vals,
									test_new_outer_state_vals,
									state_network_history);
								test_state_network_histories[iter_index][f_index][i_index] = state_network_history;
								test_new_inner_state_vals[i_index] += this->test_state_networks[f_index][i_index]->output->acti_vals[0];
							}
						}
					}

					scale_factor *= this->inner_scope_scale_mods[f_index]->weight;

					Scope* inner_scope = solution->scopes[this->existing_scope_ids[f_index]];
					int num_input_states_diff = inner_scope->num_states - this->num_inner_inputs[f_index];

					vector<double> inner_input_vals(new_inner_state_vals.begin() + this->inner_input_start_indexes[f_index],
						new_inner_state_vals.begin() + this->inner_input_start_indexes[f_index] + this->num_inner_inputs[f_index]);
					inner_input_vals.insert(inner_input_vals.end(), num_input_states_diff, 0.0);

					vector<bool> inner_inputs_initialized(this->curr_inner_inputs_needed.begin() + this->inner_input_start_indexes[f_index],
						this->curr_inner_inputs_needed.begin() + this->inner_input_start_indexes[f_index] + this->num_inner_inputs[f_index]);
					inner_inputs_initialized.insert(inner_inputs_initialized.end(), num_input_states_diff, false);

					if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE || run_helper.explore_phase == EXPLORE_PHASE_NONE) {
						inner_input_vals_snapshots[iter_index][f_index] = inner_input_vals;

						vector<double> test_inner_input_vals(test_new_inner_state_vals.begin() + this->inner_input_start_indexes[f_index],
							test_new_inner_state_vals.begin() + this->inner_input_start_indexes[f_index] + this->num_inner_inputs[f_index]);
						test_inner_input_vals_snapshots[iter_index][f_index] = test_inner_input_vals;
					}

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

					// make sure inner_scope can't explore
					int curr_explore_phase = run_helper.explore_phase;
					if (run_helper.explore_phase == EXPLORE_PHASE_NONE) {
						run_helper.explore_phase = EXPLORE_PHASE_UPDATE;
					}

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
					history->inner_scope_histories[iter_index][f_index] = scope_history;

					run_helper.explore_phase = curr_explore_phase;

					for (int i_index = 0; i_index < this->num_inner_inputs[f_index]; i_index++) {
						if (this->curr_inner_inputs_needed[this->inner_input_start_indexes[f_index] + i_index]) {
							new_inner_state_vals[this->inner_input_start_indexes[f_index] + i_index] = inner_input_vals[i_index];
						}
					}
					if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE || run_helper.explore_phase == EXPLORE_PHASE_NONE) {
						for (int i_index = 0; i_index < this->num_inner_inputs[f_index]; i_index++) {
							if (this->curr_inner_inputs_needed[this->inner_input_start_indexes[f_index] + i_index]) {
								// use inner_input_vals for test_new_inner_state_vals as well
								test_new_inner_state_vals[this->inner_input_start_indexes[f_index] + i_index] = inner_input_vals[i_index];
							}
						}
					}

					vector<double> new_state_vals;
					for (int i_index = 0; i_index < this->curr_num_new_inner_states; i_index++) {
						new_state_vals.push_back(new_inner_state_vals[this->sum_inner_inputs+i_index]);
					}
					vector<double> test_new_state_vals;
					if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE || run_helper.explore_phase == EXPLORE_PHASE_NONE) {
						for (int i_index = 0; i_index < this->curr_num_new_inner_states; i_index++) {
							test_new_state_vals.push_back(test_new_inner_state_vals[this->sum_inner_inputs+i_index]);
						}
					}
					clean_inner_scope_activate_helper(new_state_vals,
													  scope_history,
													  inner_scope_context,
													  inner_node_context,
													  run_helper,
													  iter_index,
													  f_index,
													  history,
													  test_new_state_vals,
													  test_inner_state_network_histories);
					for (int i_index = 0; i_index < this->curr_num_new_inner_states; i_index++) {
						new_inner_state_vals[this->sum_inner_inputs+i_index] = new_state_vals[i_index];
					}
					if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE || run_helper.explore_phase == EXPLORE_PHASE_NONE) {
						for (int i_index = 0; i_index < this->curr_num_new_inner_states; i_index++) {
							test_new_inner_state_vals[this->sum_inner_inputs+i_index] = test_new_state_vals[i_index];
						}
					}

					scale_factor /= this->inner_scope_scale_mods[f_index]->weight;

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

						if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE || run_helper.explore_phase == EXPLORE_PHASE_NONE) {
							if (!this->test_state_networks_not_needed[f_index][i_index]) {
								StateNetworkHistory* state_network_history = new StateNetworkHistory(this->test_state_networks[f_index][i_index]);
								this->test_state_networks[f_index][i_index]->new_sequence_activate(
									test_new_inner_state_vals,
									test_state_vals,
									test_new_outer_state_vals,
									state_network_history);
								test_state_network_histories[iter_index][f_index][i_index] = state_network_history;
								test_new_inner_state_vals[i_index] += this->test_state_networks[f_index][i_index]->output->acti_vals[0];
							}
						}
					}
					for (int s_index = 0; s_index < this->num_states; s_index++) {
						if (states_initialized[s_index]) {
							int state_index = this->sum_inner_inputs
								+ this->curr_num_new_inner_states
								+ s_index;
							if (!this->curr_state_networks_not_needed[f_index][state_index]) {
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

							if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE || run_helper.explore_phase == EXPLORE_PHASE_NONE) {
								if (!this->test_state_networks_not_needed[f_index][state_index]) {
									StateNetworkHistory* state_network_history = new StateNetworkHistory(this->test_state_networks[f_index][state_index]);
									this->test_state_networks[f_index][state_index]->new_sequence_activate(
										test_new_inner_state_vals,
										test_state_vals,
										test_new_outer_state_vals,
										state_network_history);
									test_state_network_histories[iter_index][f_index][state_index] = state_network_history;
									test_state_vals[s_index] += this->test_state_networks[f_index][state_index]->output->acti_vals[0];
								}
							}
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

						if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE || run_helper.explore_phase == EXPLORE_PHASE_NONE) {
							if (!this->test_state_networks_not_needed[f_index][i_index]) {
								StateNetworkHistory* state_network_history = new StateNetworkHistory(this->test_state_networks[f_index][i_index]);
								this->test_state_networks[f_index][i_index]->new_sequence_activate(
									obs,
									test_new_inner_state_vals,
									test_state_vals,
									test_new_outer_state_vals,
									state_network_history);
								test_state_network_histories[iter_index][f_index][i_index] = state_network_history;
								test_new_inner_state_vals[i_index] += this->test_state_networks[f_index][i_index]->output->acti_vals[0];
							}
						}
					}
					for (int s_index = 0; s_index < this->num_states; s_index++) {
						if (states_initialized[s_index]) {
							int state_index = this->sum_inner_inputs
								+ this->curr_num_new_inner_states
								+ s_index;
							if (!this->curr_state_networks_not_needed[f_index][state_index]) {
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

							if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE || run_helper.explore_phase == EXPLORE_PHASE_NONE) {
								if (!this->test_state_networks_not_needed[f_index][state_index]) {
									StateNetworkHistory* state_network_history = new StateNetworkHistory(this->test_state_networks[f_index][state_index]);
									this->test_state_networks[f_index][state_index]->new_sequence_activate(
										obs,
										test_new_inner_state_vals,
										test_state_vals,
										test_new_outer_state_vals,
										state_network_history);
									test_state_network_histories[iter_index][f_index][state_index] = state_network_history;
									test_state_vals[s_index] += this->test_state_networks[f_index][state_index]->output->acti_vals[0];
								}
							}
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

				if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE || run_helper.explore_phase == EXPLORE_PHASE_NONE) {
					StateNetworkHistory* test_score_network_history = new StateNetworkHistory(this->test_score_networks[f_index]);
					this->test_score_networks[f_index]->new_sequence_activate(
						test_new_inner_state_vals,
						test_state_vals,
						test_new_outer_state_vals,
						test_score_network_history);
					test_score_network_histories[iter_index][f_index] = test_score_network_history;
					test_score_network_updates[iter_index][f_index] = this->test_score_networks[f_index]->output->acti_vals[0];
				}
			}
		}

		iter_index++;
	}

	if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE || run_helper.explore_phase == EXPLORE_PHASE_NONE) {
		vector<double> test_new_inner_state_errors(this->sum_inner_inputs+this->curr_num_new_inner_states, 0.0);
		vector<double> test_state_errors(this->num_states);
		for (int s_index = 0; s_index < this->num_states; s_index++) {
			test_state_errors[s_index] = state_vals[s_index] - test_state_vals[s_index];
			this->sum_error += abs(test_state_errors[s_index]);
		}
		vector<double> test_new_outer_state_errors(this->curr_num_new_outer_states, 0.0);

		double target_max_update;
		if (this->state_iter <= 130000) {
			target_max_update = 0.01;
		} else {
			target_max_update = 0.002;
		}

		// ending
		this->sum_error += abs(test_continue_score_diff.back());
		this->test_continue_score_network->new_sequence_backprop(
			test_continue_score_diff.back(),
			test_new_inner_state_errors,
			test_state_errors,
			test_new_outer_state_errors,
			target_max_update,
			test_continue_score_network_histories.back());

		this->sum_error += abs(test_continue_misguess_diff.back());
		this->test_continue_misguess_network->new_sequence_backprop(
			test_continue_misguess_diff.back(),
			test_new_inner_state_errors,
			test_state_errors,
			test_new_outer_state_errors,
			target_max_update,
			test_continue_misguess_network_histories.back());

		this->sum_error += abs(test_halt_score_diff.back());
		this->test_halt_score_network->new_sequence_backprop(
			test_halt_score_diff.back(),
			test_new_inner_state_errors,
			test_state_errors,
			test_new_outer_state_errors,
			target_max_update,
			test_halt_score_network_histories.back());

		this->sum_error += abs(test_halt_misguess_diff.back());
		this->test_halt_misguess_network->new_sequence_backprop(
			test_halt_misguess_diff.back(),
			test_new_inner_state_errors,
			test_state_errors,
			test_new_outer_state_errors,
			target_max_update,
			test_halt_misguess_network_histories.back());

		for (int iter_index = history->num_loop_iters-1; iter_index >= 0; iter_index--) {
			for (int f_index = this->sequence_length-1; f_index >= 0; f_index--) {
				double test_score_network_error = history->score_network_updates[iter_index][f_index]
					- test_score_network_updates[iter_index][f_index];
				this->sum_error += abs(test_score_network_error);
				this->test_score_networks[f_index]->new_sequence_backprop(
					test_score_network_error,
					test_new_inner_state_errors,
					test_state_errors,
					test_new_outer_state_errors,
					target_max_update,
					test_score_network_histories[iter_index][f_index]);

				if (this->is_inner_scope[f_index]) {
					for (int s_index = this->num_states-1; s_index >= 0; s_index--) {
						if (states_initialized[s_index]) {
							int state_index = this->sum_inner_inputs
								+ this->curr_num_new_inner_states
								+ s_index;
							if (!this->test_state_networks_not_needed[f_index][state_index]) {
								this->test_state_networks[f_index][state_index]->new_sequence_backprop(
									test_state_errors[s_index],
									test_new_inner_state_errors,
									test_state_errors,
									test_new_outer_state_errors,
									target_max_update,
									test_state_network_histories[iter_index][f_index][state_index]);
							}
						}
					}
					for (int i_index = this->sum_inner_inputs+this->curr_num_new_inner_states-1;
							i_index >= this->inner_input_start_indexes[f_index] + this->num_inner_inputs[f_index]; i_index--) {
						if (!this->test_state_networks_not_needed[f_index][i_index]) {
							this->test_state_networks[f_index][i_index]->new_sequence_backprop(
								test_new_inner_state_errors[i_index],
								test_new_inner_state_errors,
								test_state_errors,
								test_new_outer_state_errors,
								target_max_update,
								test_state_network_histories[iter_index][f_index][i_index]);
						}
					}

					vector<double> test_new_state_errors;
					for (int i_index = 0; i_index < this->curr_num_new_inner_states; i_index++) {
						test_new_state_errors.push_back(test_new_inner_state_errors[this->sum_inner_inputs+i_index]);
					}
					for (int n_index = (int)test_inner_state_network_histories[iter_index][f_index].size()-1; n_index >= 0; n_index--) {
						for (int i_index = this->curr_num_new_inner_states-1; i_index >= 0; i_index--) {
							if (test_inner_state_network_histories[iter_index][f_index][n_index][i_index] != NULL) {
								StateNetwork* state_network = test_inner_state_network_histories[iter_index][f_index][n_index][i_index]->network;
								state_network->new_external_backprop(
									test_new_state_errors[i_index],
									test_new_state_errors,
									target_max_update,
									test_inner_state_network_histories[iter_index][f_index][n_index][i_index]);
							}
						}
					}
					for (int i_index = 0; i_index < this->curr_num_new_inner_states; i_index++) {
						test_new_inner_state_errors[this->sum_inner_inputs+i_index] = test_new_state_errors[i_index];
					}

					for (int i_index = 0; i_index < this->num_inner_inputs[f_index]; i_index++) {
						if (this->curr_inner_inputs_needed[this->inner_input_start_indexes[f_index] + i_index]) {
							double inner_input_error = inner_input_vals_snapshots[iter_index][f_index][i_index]
								- test_inner_input_vals_snapshots[iter_index][f_index][i_index];
							this->sum_error += abs(inner_input_error);
							test_new_inner_state_errors[this->inner_input_start_indexes[f_index] + i_index] = inner_input_error;
							// set instead of sum as not connected
						}
					}

					for (int i_index = this->inner_input_start_indexes[f_index]+this->num_inner_inputs[f_index]-1; i_index >= 0; i_index--) {
						if (!this->test_state_networks_not_needed[f_index][i_index]) {
							this->test_state_networks[f_index][i_index]->new_sequence_backprop(
								test_new_inner_state_errors[i_index],
								test_new_inner_state_errors,
								test_state_errors,
								test_new_outer_state_errors,
								target_max_update,
								test_state_network_histories[iter_index][f_index][i_index]);
						}
					}
				} else {
					for (int s_index = this->num_states-1; s_index >= 0; s_index--) {
						if (states_initialized[s_index]) {
							int state_index = this->sum_inner_inputs
								+ this->curr_num_new_inner_states
								+ s_index;
							if (!this->test_state_networks_not_needed[f_index][state_index]) {
								this->test_state_networks[f_index][state_index]->new_sequence_backprop(
									test_state_errors[s_index],
									test_new_inner_state_errors,
									test_state_errors,
									test_new_outer_state_errors,
									target_max_update,
									test_state_network_histories[iter_index][f_index][state_index]);
							}
						}
					}
					for (int i_index = this->sum_inner_inputs+this->curr_num_new_inner_states-1; i_index >= 0; i_index--) {
						if (!this->test_state_networks_not_needed[f_index][i_index]) {
							this->test_state_networks[f_index][i_index]->new_sequence_backprop(
								test_new_inner_state_errors[i_index],
								test_new_inner_state_errors,
								test_state_errors,
								test_new_outer_state_errors,
								target_max_update,
								test_state_network_histories[iter_index][f_index][i_index]);
						}
					}
				}
			}

			this->sum_error += abs(test_continue_score_diff[iter_index]);
			this->test_continue_score_network->new_sequence_backprop(
				test_continue_score_diff[iter_index],
				test_new_inner_state_errors,
				test_state_errors,
				test_new_outer_state_errors,
				target_max_update,
				test_continue_score_network_histories[iter_index]);

			this->sum_error += abs(test_continue_misguess_diff[iter_index]);
			this->test_continue_misguess_network->new_sequence_backprop(
				test_continue_misguess_diff[iter_index],
				test_new_inner_state_errors,
				test_state_errors,
				test_new_outer_state_errors,
				target_max_update,
				test_continue_misguess_network_histories[iter_index]);

			this->sum_error += abs(test_halt_score_diff[iter_index]);
			this->test_halt_score_network->new_sequence_backprop(
				test_halt_score_diff[iter_index],
				test_new_inner_state_errors,
				test_state_errors,
				test_new_outer_state_errors,
				target_max_update,
				test_halt_score_network_histories[iter_index]);

			this->sum_error += abs(test_halt_misguess_diff[iter_index]);
			this->test_halt_misguess_network->new_sequence_backprop(
				test_halt_misguess_diff[iter_index],
				test_new_inner_state_errors,
				test_state_errors,
				test_new_outer_state_errors,
				target_max_update,
				test_halt_misguess_network_histories[iter_index]);
		}

		for (int i_index = this->sum_inner_inputs+this->curr_num_new_inner_states-1; i_index >= 0; i_index--) {
			if (i_index >= this->sum_inner_inputs
					|| this->curr_inner_inputs_needed[i_index]) {
				this->test_starting_state_networks[i_index]->new_sequence_backprop(
					test_new_inner_state_errors[i_index],
					test_new_inner_state_errors,
					test_state_errors,
					test_new_outer_state_errors,
					target_max_update,
					test_starting_state_network_histories[i_index]);
			}
		}

		for (int i_index = 0; i_index < (int)test_starting_state_network_histories.size(); i_index++) {
			if (test_starting_state_network_histories[i_index] != NULL) {
				delete test_starting_state_network_histories[i_index];
			}
		}

		for (int iter_index = 0; iter_index < (int)test_continue_score_network_histories.size(); iter_index++) {
			if (test_continue_score_network_histories[iter_index] != NULL) {
				delete test_continue_score_network_histories[iter_index];
			}
		}

		for (int iter_index = 0; iter_index < (int)test_continue_misguess_network_histories.size(); iter_index++) {
			if (test_continue_misguess_network_histories[iter_index] != NULL) {
				delete test_continue_misguess_network_histories[iter_index];
			}
		}

		for (int iter_index = 0; iter_index < (int)test_halt_score_network_histories.size(); iter_index++) {
			if (test_halt_score_network_histories[iter_index] != NULL) {
				delete test_halt_score_network_histories[iter_index];
			}
		}

		for (int iter_index = 0; iter_index < (int)test_halt_misguess_network_histories.size(); iter_index++) {
			if (test_halt_misguess_network_histories[iter_index] != NULL) {
				delete test_halt_misguess_network_histories[iter_index];
			}
		}

		for (int iter_index = 0; iter_index < (int)test_state_network_histories.size(); iter_index++) {
			for (int f_index = 0; f_index < (int)test_state_network_histories[iter_index].size(); f_index++) {
				for (int s_index = 0; s_index < (int)test_state_network_histories[iter_index][f_index].size(); s_index++) {
					if (test_state_network_histories[iter_index][f_index][s_index] != NULL) {
						delete test_state_network_histories[iter_index][f_index][s_index];
					}
				}
			}
		}

		for (int iter_index = 0; iter_index < (int)test_inner_scope_histories.size(); iter_index++) {
			for (int f_index = 0; f_index < (int)test_inner_scope_histories[iter_index].size(); f_index++) {
				if (test_inner_scope_histories[iter_index][f_index] != NULL) {
					delete test_inner_scope_histories[iter_index][f_index];
				}
			}
		}

		for (int iter_index = 0; iter_index < (int)test_score_network_histories.size(); iter_index++) {
			for (int f_index = 0; f_index < (int)test_score_network_histories[iter_index].size(); f_index++) {
				if (test_score_network_histories[iter_index][f_index] != NULL) {
					delete test_score_network_histories[iter_index][f_index];
				}
			}
		}

		for (int iter_index = 0; iter_index < (int)test_inner_state_network_histories.size(); iter_index++) {
			for (int f_index = 0; f_index < (int)test_inner_state_network_histories[iter_index].size(); f_index++) {
				for (int n_index = 0; n_index < (int)test_inner_state_network_histories[iter_index][f_index].size(); n_index++) {
					for (int s_index = 0; s_index < (int)test_inner_state_network_histories[iter_index][f_index][n_index].size(); s_index++) {
						if (test_inner_state_network_histories[iter_index][f_index][n_index][s_index] != NULL) {
							delete test_inner_state_network_histories[iter_index][f_index][n_index][s_index];
						}
					}
				}
			}
		}

		this->state_iter++;
		this->sub_iter++;
	}
}

void LoopFold::clean_backprop(vector<double>& state_errors,
							  vector<bool>& states_initialized,
							  double target_val,
							  double final_misguess,
							  double final_sum_impact,
							  double& predicted_score,
							  double& scale_factor,
							  double& scale_factor_error,
							  RunHelper& run_helper,
							  LoopFoldHistory* history) {
	if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN) {
		vector<double> new_inner_state_errors(this->sum_inner_inputs+this->curr_num_new_inner_states, 0.0);
		vector<double> new_outer_state_errors(this->curr_num_new_outer_states, 0.0);

		double halt_predicted_score_error = target_val - predicted_score;
		this->curr_halt_score_network->new_sequence_backprop_errors_with_no_weight_change(
			scale_factor*halt_predicted_score_error,
			new_inner_state_errors,
			state_errors,
			new_outer_state_errors,
			history->halt_score_network_history);

		predicted_score -= scale_factor*history->halt_score_network_update;

		this->curr_halt_misguess_network->new_sequence_backprop_errors_with_no_weight_change(
			final_misguess - history->halt_misguess_val,
			new_inner_state_errors,
			state_errors,
			new_outer_state_errors,
			history->halt_misguess_network_history);

		for (int iter_index = history->num_loop_iters-1; iter_index >= 0; iter_index--) {
			for (int f_index = this->sequence_length-1; f_index >= 0; f_index--) {
				double predicted_score_error = target_val - predicted_score;
				this->curr_score_networks[f_index]->new_sequence_backprop_errors_with_no_weight_change(
					scale_factor*predicted_score_error,
					new_inner_state_errors,
					state_errors,
					new_outer_state_errors,
					history->score_network_histories[iter_index][f_index]);

				predicted_score -= scale_factor*history->score_network_updates[iter_index][f_index];

				if (this->is_inner_scope[f_index]) {
					for (int s_index = this->num_states-1; s_index >= 0; s_index--) {
						if (states_initialized[s_index]) {
							int state_index = this->sum_inner_inputs
								+ this->curr_num_new_inner_states
								+ s_index;
							if (!this->curr_state_networks_not_needed[f_index][state_index]) {
								this->curr_state_networks[f_index][state_index]->new_sequence_backprop_errors_with_no_weight_change(
									state_errors[s_index],
									new_inner_state_errors,
									state_errors,
									new_outer_state_errors,
									history->state_network_histories[iter_index][f_index][state_index]);
							}
						}
					}
					for (int i_index = this->sum_inner_inputs+this->curr_num_new_inner_states-1;
							i_index >= this->inner_input_start_indexes[f_index] + this->num_inner_inputs[f_index]; i_index--) {
						if (!this->curr_state_networks_not_needed[f_index][i_index]) {
							this->curr_state_networks[f_index][i_index]->new_sequence_backprop_errors_with_no_weight_change(
								new_inner_state_errors[i_index],
								new_inner_state_errors,
								state_errors,
								new_outer_state_errors,
								history->state_network_histories[iter_index][f_index][i_index]);
						}
					}

					scale_factor *= this->inner_scope_scale_mods[f_index]->weight;

					vector<double> new_state_errors;
					for (int i_index = 0; i_index < this->curr_num_new_inner_states; i_index++) {
						new_state_errors.push_back(new_inner_state_errors[this->sum_inner_inputs+i_index]);
					}
					for (int n_index = (int)history->inner_state_network_histories[iter_index][f_index].size()-1; n_index >= 0; n_index--) {
						for (int i_index = this->curr_num_new_inner_states-1; i_index >= 0; i_index--) {
							if (history->inner_state_network_histories[iter_index][f_index][n_index][i_index] != NULL) {
								StateNetwork* state_network = history->inner_state_network_histories[iter_index][f_index][n_index][i_index]->network;
								state_network->new_external_backprop_errors_with_no_weight_change(
									new_state_errors[i_index],
									new_state_errors,
									history->inner_state_network_histories[iter_index][f_index][n_index][i_index]);
							}
						}
					}
					for (int i_index = 0; i_index < this->curr_num_new_inner_states; i_index++) {
						new_inner_state_errors[this->sum_inner_inputs+i_index] = new_state_errors[i_index];
					}

					Scope* inner_scope = solution->scopes[this->existing_scope_ids[f_index]];
					int num_input_states_diff = inner_scope->num_states - this->num_inner_inputs[f_index];

					vector<double> inner_input_errors(new_inner_state_errors.begin() + this->inner_input_start_indexes[f_index],
						new_inner_state_errors.begin() + this->inner_input_start_indexes[f_index] + this->num_inner_inputs[f_index]);
					inner_input_errors.insert(inner_input_errors.end(), num_input_states_diff, 0.0);

					vector<bool> inner_inputs_initialized(this->curr_inner_inputs_needed.begin() + this->inner_input_start_indexes[f_index],
						this->curr_inner_inputs_needed.begin() + this->inner_input_start_indexes[f_index] + this->num_inner_inputs[f_index]);
					inner_inputs_initialized.insert(inner_inputs_initialized.end(), num_input_states_diff, false);

					double scope_scale_factor_error = 0.0;	// unused
					inner_scope->backprop(inner_input_errors,
										  inner_inputs_initialized,
										  target_val,
										  final_misguess,
										  final_sum_impact,
										  predicted_score,
										  scale_factor,
										  scope_scale_factor_error,
										  run_helper,
										  history->inner_scope_histories[iter_index][f_index]);

					for (int i_index = 0; i_index < this->num_inner_inputs[f_index]; i_index++) {
						if (this->curr_inner_inputs_needed[this->inner_input_start_indexes[f_index] + i_index]) {
							new_inner_state_errors[this->inner_input_start_indexes[f_index] + i_index] = inner_input_errors[i_index];
						}
					}

					scale_factor /= this->inner_scope_scale_mods[f_index]->weight;

					for (int i_index = this->inner_input_start_indexes[f_index]+this->num_inner_inputs[f_index]-1; i_index >= 0; i_index--) {
						if (!this->curr_state_networks_not_needed[f_index][i_index]) {
							this->curr_state_networks[f_index][i_index]->new_sequence_backprop_errors_with_no_weight_change(
								new_inner_state_errors[i_index],
								new_inner_state_errors,
								state_errors,
								new_outer_state_errors,
								history->state_network_histories[iter_index][f_index][i_index]);
						}
					}
				} else {
					for (int s_index = this->num_states-1; s_index >= 0; s_index--) {
						if (states_initialized[s_index]) {
							int state_index = this->sum_inner_inputs
								+ this->curr_num_new_inner_states
								+ s_index;
							if (!this->curr_state_networks_not_needed[f_index][state_index]) {
								this->curr_state_networks[f_index][state_index]->new_sequence_backprop_errors_with_no_weight_change(
									state_errors[s_index],
									new_inner_state_errors,
									state_errors,
									new_outer_state_errors,
									history->state_network_histories[iter_index][f_index][state_index]);
							}
						}
					}
					for (int i_index = this->sum_inner_inputs+this->curr_num_new_inner_states-1; i_index >= 0; i_index--) {
						if (!this->curr_state_networks_not_needed[f_index][i_index]) {
							this->curr_state_networks[f_index][i_index]->new_sequence_backprop_errors_with_no_weight_change(
								new_inner_state_errors[i_index],
								new_inner_state_errors,
								state_errors,
								new_outer_state_errors,
								history->state_network_histories[iter_index][f_index][i_index]);
						}
					}
				}
			}

			double continue_predicted_score_error = target_val - predicted_score;
			this->curr_continue_score_network->new_sequence_backprop_errors_with_no_weight_change(
				scale_factor*continue_predicted_score_error,
				new_inner_state_errors,
				state_errors,
				new_outer_state_errors,
				history->continue_score_network_histories[iter_index]);

			predicted_score -= scale_factor*history->continue_score_network_updates[iter_index];

			this->curr_continue_misguess_network->new_sequence_backprop_errors_with_no_weight_change(
				final_misguess - history->continue_misguess_vals[iter_index],
				new_inner_state_errors,
				state_errors,
				new_outer_state_errors,
				history->continue_misguess_network_histories[iter_index]);
		}

		for (int i_index = this->sum_inner_inputs+this->curr_num_new_inner_states-1; i_index >= 0; i_index--) {
			if (i_index >= this->sum_inner_inputs
					|| this->curr_inner_inputs_needed[i_index]) {
				this->curr_starting_state_networks[i_index]->new_sequence_backprop_errors_with_no_weight_change(
					new_inner_state_errors[i_index],
					new_inner_state_errors,
					state_errors,
					new_outer_state_errors,
					history->starting_state_network_histories[i_index]);
			}
		}
	} else if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE) {
		this->curr_average_score = 0.9999*this->curr_average_score + 0.0001*target_val;
		double score_variance = (this->curr_average_score - target_val)*(this->curr_average_score - target_val);
		this->curr_score_variance = 0.9999*this->curr_score_variance + 0.0001*score_variance;
		this->curr_average_misguess = 0.9999*this->curr_average_misguess + 0.0001*final_misguess;
		double misguess_variance = (this->curr_average_misguess - final_misguess)*(this->curr_average_misguess - final_misguess);
		this->curr_misguess_variance = 0.9999*this->curr_misguess_variance + 0.0001*misguess_variance;

		if (history->state_iter_snapshot <= this->state_iter) {
			double halt_predicted_score_error = target_val - predicted_score;
			this->curr_halt_score_network->backprop_weights_with_no_error_signal(
				scale_factor*halt_predicted_score_error,
				0.002,
				history->halt_score_network_history);

			predicted_score -= scale_factor*history->halt_score_network_update;

			this->curr_halt_misguess_network->backprop_weights_with_no_error_signal(
				final_misguess - history->halt_misguess_val,
				0.002,
				history->halt_misguess_network_history);

			for (int iter_index = history->num_loop_iters-1; iter_index >= 0; iter_index--) {
				for (int f_index = this->sequence_length-1; f_index >= 0; f_index--) {
					double predicted_score_error = target_val - predicted_score;
					this->curr_score_networks[f_index]->backprop_weights_with_no_error_signal(
						scale_factor*predicted_score_error,
						0.002,
						history->score_network_histories[iter_index][f_index]);

					predicted_score -= scale_factor*history->score_network_updates[iter_index][f_index];

					if (this->is_inner_scope[f_index]) {
						scale_factor *= this->inner_scope_scale_mods[f_index]->weight;

						Scope* inner_scope = solution->scopes[this->existing_scope_ids[f_index]];

						// unused
						vector<double> inner_input_errors;
						vector<bool> inner_inputs_initialized;

						double scope_scale_factor_error = 0.0;
						inner_scope->backprop(inner_input_errors,
											  inner_inputs_initialized,
											  target_val,
											  final_misguess,
											  final_sum_impact,
											  predicted_score,
											  scale_factor,
											  scope_scale_factor_error,
											  run_helper,
											  history->inner_scope_histories[iter_index][f_index]);

						scale_factor /= this->inner_scope_scale_mods[f_index]->weight;

						this->inner_scope_scale_mods[f_index]->backprop(scope_scale_factor_error, 0.0002);

						scale_factor_error += this->inner_scope_scale_mods[f_index]->weight*scope_scale_factor_error;
					}
				}

				double continue_predicted_score_error = target_val - predicted_score;
				this->curr_continue_score_network->backprop_weights_with_no_error_signal(
					scale_factor*continue_predicted_score_error,
					0.002,
					history->continue_score_network_histories[iter_index]);

				predicted_score -= scale_factor*history->continue_score_network_updates[iter_index];

				this->curr_continue_misguess_network->backprop_weights_with_no_error_signal(
					final_misguess - history->continue_misguess_vals[iter_index],
					0.002,
					history->continue_misguess_network_histories[iter_index]);
			}

			clean_increment();
		}
	}
}
