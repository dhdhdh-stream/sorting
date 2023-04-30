#include "fold.h"

#include <iostream>

#include "action_node.h"
#include "constants.h"
#include "globals.h"
#include "scope_node.h"

using namespace std;

void Fold::clean_outer_scope_activate_helper(vector<double>& new_outer_state_vals,
											 ScopeHistory* scope_history,
											 FoldHistory* history) {
	int scope_id = scope_history->scope->id;

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
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];
				clean_outer_scope_activate_helper(new_outer_state_vals,
												  scope_node_history->inner_scope_history,
												  history);
			}
		}
	}
}

void Fold::clean_score_activate(vector<double>& state_vals,
								vector<ScopeHistory*> context_histories,
								FoldHistory* history) {
	vector<double> new_outer_state_vals(this->curr_num_new_outer_states, 0.0);

	ScopeHistory* scope_history = context_histories[context_histories.size() - this->scope_context.size()];
	clean_outer_scope_activate_helper(new_outer_state_vals,
									  scope_history,
									  history);

	StateNetworkHistory* starting_score_network_history = new StateNetworkHistory(this->curr_starting_score_network);
	this->curr_starting_score_network->new_external_activate(
		state_vals,
		new_outer_state_vals,
		starting_score_network_history);
	history->starting_score_update = this->curr_starting_score_network->output->acti_vals[0];
	history->starting_score_network_history = starting_score_network_history;

	history->new_outer_state_vals = new_outer_state_vals;

	// modify predicted_score if path taken outside
}

void Fold::clean_inner_scope_activate_helper(vector<double>& new_state_vals,
											 vector<bool>& new_state_vals_initialized,
											 ScopeHistory* scope_history,
											 vector<int>& curr_scope_context,
											 vector<int>& curr_node_context,
											 RunHelper& run_helper,
											 int step_index,
											 FoldHistory* history,
											 vector<double>& test_new_state_vals,
											 vector<bool>& test_new_state_vals_initialized,
											 vector<vector<vector<StateNetworkHistory*>>>& test_inner_state_network_histories) {
	int scope_id = scope_history->scope->id;
	curr_scope_context.push_back(scope_id);
	curr_node_context.push_back(-1);

	map<int, vector<vector<StateNetwork*>>>::iterator it = this->curr_inner_state_networks.find(scope_id);

	map<int, vector<vector<StateNetwork*>>>::iterator test_it;
	if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE || run_helper.explore_phase == EXPLORE_PHASE_NONE) {
		test_it = this->test_inner_state_networks.find(scope_id);

		if (this->state == FOLD_STATE_REMOVE_INNER_SCOPE) {
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
							history->inner_state_network_histories[step_index].push_back(vector<StateNetworkHistory*>());
							for (int s_index = 0; s_index < this->curr_num_new_inner_states; s_index++) {
								bool is_needed = false;
								if (new_state_vals_initialized[s_index]) {
									if (this->state == FOLD_STATE_REMOVE_INNER_SCOPE
											|| !this->curr_inner_state_networks_not_needed[scope_id][node_id][s_index]) {
										is_needed = true;
									}
								}
								if (is_needed) {
									StateNetworkHistory* state_network_history = new StateNetworkHistory(it->second[node_id][s_index]);
									it->second[node_id][s_index]->new_external_activate(
										action_node_history->obs_snapshot,
										action_node_history->ending_state_snapshot,
										new_state_vals,
										state_network_history);
									history->inner_state_network_histories[step_index].back().push_back(state_network_history);
									new_state_vals[s_index] += it->second[node_id][s_index]->output->acti_vals[0];
								} else {
									history->inner_state_network_histories[step_index].back().push_back(NULL);
								}
							}
						} else {
							for (int s_index = 0; s_index < this->curr_num_new_inner_states; s_index++) {
								bool is_needed = false;
								if (new_state_vals_initialized[s_index]) {
									if (this->state == FOLD_STATE_REMOVE_INNER_SCOPE
											|| !this->curr_inner_state_networks_not_needed[scope_id][node_id][s_index]) {
										is_needed = true;
									}
								}
								if (is_needed) {
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
							test_inner_state_network_histories[step_index].push_back(vector<StateNetworkHistory*>());
							ActionNodeHistory* action_node_history = (ActionNodeHistory*)scope_history->node_histories[i_index][h_index];
							for (int s_index = 0; s_index < this->curr_num_new_inner_states; s_index++) {
								bool is_needed = false;
								if (test_new_state_vals_initialized[s_index]) {
									if (this->state == FOLD_STATE_REMOVE_INNER_SCOPE) {
										is_needed = true;
									} else {
										if (this->state == FOLD_STATE_REMOVE_INNER_SCOPE_NETWORK) {
											is_needed = !this->test_inner_state_networks_not_needed[scope_id][node_id][s_index];
										} else {
											is_needed = !this->curr_inner_state_networks_not_needed[scope_id][node_id][s_index];
										}
									}
								}
								if (is_needed) {
									StateNetworkHistory* state_network_history = new StateNetworkHistory(test_it->second[node_id][s_index]);
									test_it->second[node_id][s_index]->new_external_activate(
										action_node_history->obs_snapshot,
										action_node_history->ending_state_snapshot,
										test_new_state_vals,
										state_network_history);
									test_inner_state_network_histories[step_index].back().push_back(state_network_history);
									test_new_state_vals[s_index] += test_it->second[node_id][s_index]->output->acti_vals[0];
								} else {
									test_inner_state_network_histories[step_index].back().push_back(NULL);
								}
							}
						}
					}
				}
			} else if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_INNER_SCOPE) {
				curr_node_context.back() = scope_history->node_histories[i_index][h_index]->scope_index;

				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];
				clean_inner_scope_activate_helper(new_state_vals,
												  new_state_vals_initialized,
												  scope_node_history->inner_scope_history,
												  curr_scope_context,
												  curr_node_context,
												  run_helper,
												  step_index,
												  history,
												  test_new_state_vals,
												  test_new_state_vals_initialized,
												  test_inner_state_network_histories);

				curr_node_context.back() = -1;
			}
		}
	}

	curr_scope_context.pop_back();
	curr_node_context.pop_back();
}

void Fold::clean_sequence_activate(Problem& problem,
								   vector<double>& state_vals,
								   vector<bool>& states_initialized,
								   double& predicted_score,
								   double& scale_factor,
								   double& sum_impact,
								   RunHelper& run_helper,
								   FoldHistory* history) {
	vector<double> new_inner_state_vals(this->sum_inner_inputs + this->curr_num_new_inner_states, 0.0);
	vector<double> new_outer_state_vals = history->new_outer_state_vals;

	if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN) {
		int num_inner_networks = this->sum_inner_inputs
			+ this->curr_num_new_inner_states
			+ this->num_sequence_states;
		history->state_network_histories = vector<vector<StateNetworkHistory*>>(
			this->sequence_length, vector<StateNetworkHistory*>(num_inner_networks, NULL));
	}
	history->inner_scope_histories = vector<ScopeHistory*>(this->sequence_length, NULL);
	history->score_network_updates = vector<double>(this->sequence_length);
	history->score_network_histories = vector<StateNetworkHistory*>(this->sequence_length, NULL);

	vector<double> test_new_inner_state_vals;
	vector<double> test_state_vals;
	vector<double> test_new_outer_state_vals;
	vector<vector<double>> inner_input_vals_snapshots;
	vector<vector<double>> test_inner_input_vals_snapshots;
	vector<vector<vector<StateNetworkHistory*>>> test_inner_state_network_histories;
	if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE || run_helper.explore_phase == EXPLORE_PHASE_NONE) {
		test_new_inner_state_vals = vector<double>(this->sum_inner_inputs + this->curr_num_new_inner_states, 0.0);
		test_state_vals = state_vals;
		test_new_outer_state_vals = history->new_outer_state_vals;
		inner_input_vals_snapshots = vector<vector<double>>(this->sequence_length);
		test_inner_input_vals_snapshots = vector<vector<double>>(this->sequence_length);
		test_inner_state_network_histories = vector<vector<vector<StateNetworkHistory*>>>(this->sequence_length);
	}

	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		if (this->is_inner_scope[f_index]) {
			for (int i_index = 0; i_index < this->inner_input_start_indexes[f_index] + this->num_inner_inputs[f_index]; i_index++) {
				// state_networks_not_needed already set in correspondence with inner_inputs_needed
				if (!this->curr_state_networks_not_needed[f_index][i_index]) {
					if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN) {
						StateNetworkHistory* state_network_history = new StateNetworkHistory(this->curr_state_networks[f_index][i_index]);
						this->curr_state_networks[f_index][i_index]->new_sequence_activate(
							new_inner_state_vals,
							state_vals,
							new_outer_state_vals,
							state_network_history);
						history->state_network_histories[f_index][i_index] = state_network_history;
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
						this->test_state_networks[f_index][i_index]->new_sequence_activate(
							test_new_inner_state_vals,
							test_state_vals,
							test_new_outer_state_vals);
						test_new_inner_state_vals[i_index] += this->test_state_networks[f_index][i_index]->output->acti_vals[0];
					}
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

			if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE || run_helper.explore_phase == EXPLORE_PHASE_NONE) {
				inner_input_vals_snapshots[f_index] = inner_input_vals;

				vector<double> test_inner_input_vals(test_new_inner_state_vals.begin() + this->inner_input_start_indexes[f_index],
					test_new_inner_state_vals.begin() + this->inner_input_start_indexes[f_index] + this->num_inner_inputs[f_index]);
				test_inner_input_vals_snapshots[f_index] = test_inner_input_vals;
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
			history->inner_scope_histories[f_index] = scope_history;

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
			vector<bool> new_state_vals_initialized;
			for (int i_index = 0; i_index < this->curr_num_new_inner_states; i_index++) {
				if (this->curr_state_not_needed_locally[f_index][this->sum_inner_inputs+i_index]) {
					new_state_vals.push_back(0.0);
					new_state_vals_initialized.push_back(false);
				} else {
					new_state_vals.push_back(new_inner_state_vals[this->sum_inner_inputs+i_index]);
					new_state_vals_initialized.push_back(true);
				}
			}
			vector<double> test_new_state_vals;
			vector<bool> test_new_state_vals_initialized;
			if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE || run_helper.explore_phase == EXPLORE_PHASE_NONE) {
				for (int i_index = 0; i_index < this->curr_num_new_inner_states; i_index++) {
					if (this->test_state_not_needed_locally[f_index][this->sum_inner_inputs+i_index]) {
						test_new_state_vals.push_back(0.0);
						test_new_state_vals_initialized.push_back(false);
					} else {
						test_new_state_vals.push_back(test_new_inner_state_vals[this->sum_inner_inputs+i_index]);
						test_new_state_vals_initialized.push_back(true);
					}
				}
			}
			clean_inner_scope_activate_helper(new_state_vals,
											  new_state_vals_initialized,
											  scope_history,
											  inner_scope_context,	// empty again
											  inner_node_context,	// empty again
											  run_helper,
											  f_index,
											  history,
											  test_new_state_vals,
											  test_new_state_vals_initialized,
											  test_inner_state_network_histories);
			for (int i_index = 0; i_index < this->curr_num_new_inner_states; i_index++) {
				if (!this->curr_state_not_needed_locally[f_index][this->sum_inner_inputs+i_index]) {
					new_inner_state_vals[this->sum_inner_inputs+i_index] = new_state_vals[i_index];
				}
			}
			if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE || run_helper.explore_phase == EXPLORE_PHASE_NONE) {
				for (int i_index = 0; i_index < this->curr_num_new_inner_states; i_index++) {
					if (!this->test_state_not_needed_locally[f_index][this->sum_inner_inputs+i_index]) {
						test_new_inner_state_vals[this->sum_inner_inputs+i_index] = test_new_state_vals[i_index];
					}
				}
			}

			// update back state so have chance to compress front after
			for (int i_index = this->inner_input_start_indexes[f_index] + this->num_inner_inputs[f_index];
					i_index < this->sum_inner_inputs + this->curr_num_new_inner_states; i_index++) {
				// state_networks_not_needed already set in correspondence with inner_inputs_needed
				if (!this->curr_state_networks_not_needed[f_index][i_index]) {
					if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN) {
						StateNetworkHistory* state_network_history = new StateNetworkHistory(this->curr_state_networks[f_index][i_index]);
						this->curr_state_networks[f_index][i_index]->new_sequence_activate(
							new_inner_state_vals,
							state_vals,
							new_outer_state_vals,
							state_network_history);
						history->state_network_histories[f_index][i_index] = state_network_history;
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
						this->test_state_networks[f_index][i_index]->new_sequence_activate(
							test_new_inner_state_vals,
							test_state_vals,
							test_new_outer_state_vals);
						test_new_inner_state_vals[i_index] += this->test_state_networks[f_index][i_index]->output->acti_vals[0];
					}
				}
			}
			for (int s_index = 0; s_index < this->num_sequence_states; s_index++) {
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
							history->state_network_histories[f_index][state_index] = state_network_history;
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
							this->test_state_networks[f_index][state_index]->new_sequence_activate(
								test_new_inner_state_vals,
								test_state_vals,
								test_new_outer_state_vals);
							test_state_vals[s_index] += this->test_state_networks[f_index][state_index]->output->acti_vals[0];
						}
					}
				}
			}
		} else {
			problem.perform_action(this->actions[f_index]);
			double obs = problem.get_observation();

			for (int i_index = 0; i_index < this->sum_inner_inputs + this->curr_num_new_inner_states; i_index++) {
				// state_networks_not_needed already set in correspondence with inner_inputs_needed
				if (!this->curr_state_networks_not_needed[f_index][i_index]) {
					if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN) {
						StateNetworkHistory* state_network_history = new StateNetworkHistory(this->curr_state_networks[f_index][i_index]);
						this->curr_state_networks[f_index][i_index]->new_sequence_activate(
							obs,
							new_inner_state_vals,
							state_vals,
							new_outer_state_vals,
							state_network_history);
						history->state_network_histories[f_index][i_index] = state_network_history;
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
						this->test_state_networks[f_index][i_index]->new_sequence_activate(
							obs,
							test_new_inner_state_vals,
							test_state_vals,
							test_new_outer_state_vals);
						test_new_inner_state_vals[i_index] += this->test_state_networks[f_index][i_index]->output->acti_vals[0];
					}
				}
			}
			for (int s_index = 0; s_index < this->num_sequence_states; s_index++) {
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
							history->state_network_histories[f_index][state_index] = state_network_history;
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
							this->test_state_networks[f_index][state_index]->new_sequence_activate(
								obs,
								test_new_inner_state_vals,
								test_state_vals,
								test_new_outer_state_vals);
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
		history->score_network_histories[f_index] = score_network_history;
		history->score_network_updates[f_index] = this->curr_score_networks[f_index]->output->acti_vals[0];
		predicted_score += scale_factor*this->curr_score_networks[f_index]->output->acti_vals[0];

		if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE || run_helper.explore_phase == EXPLORE_PHASE_NONE) {
			this->test_score_networks[f_index]->new_sequence_activate(
				test_new_inner_state_vals,
				test_state_vals,
				test_new_outer_state_vals);
		}

		for (int i_index = 0; i_index < this->curr_num_states_cleared[f_index]; i_index++) {
			new_inner_state_vals[i_index] = 0.0;
		}
		if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE || run_helper.explore_phase == EXPLORE_PHASE_NONE) {
			for (int i_index = 0; i_index < this->test_num_states_cleared[f_index]; i_index++) {
				test_new_inner_state_vals[i_index] = 0.0;
			}
		}
	}

	if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE || run_helper.explore_phase == EXPLORE_PHASE_NONE) {
		vector<double> test_new_inner_state_errors(this->sum_inner_inputs+this->curr_num_new_inner_states, 0.0);
		vector<double> test_state_errors(this->num_sequence_states);
		for (int s_index = 0; s_index < this->num_sequence_states; s_index++) {
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

		for (int f_index = this->sequence_length-1; f_index >= 0; f_index--) {
			for (int i_index = 0; i_index < this->test_num_states_cleared[f_index]; i_index++) {
				test_new_inner_state_errors[i_index] = 0.0;
			}

			double test_score_network_error = history->score_network_updates[f_index]
				- this->test_score_networks[f_index]->output->acti_vals[0];
			this->sum_error += abs(test_score_network_error);
			this->test_score_networks[f_index]->new_sequence_backprop(
				test_score_network_error,
				test_new_inner_state_errors,
				test_state_errors,
				test_new_outer_state_errors,
				target_max_update);

			if (this->is_inner_scope[f_index]) {
				for (int s_index = this->num_sequence_states-1; s_index >= 0; s_index--) {
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
								target_max_update);
						}
					}
				}
				for (int i_index = this->sum_inner_inputs+this->curr_num_new_inner_states-1;
						i_index >= this->inner_input_start_indexes[f_index] + this->num_inner_inputs[f_index]; i_index--) {
					// state_networks_not_needed already set in correspondence with inner_inputs_needed
					if (!this->test_state_networks_not_needed[f_index][i_index]) {
						this->test_state_networks[f_index][i_index]->new_sequence_backprop(
							test_new_inner_state_errors[i_index],
							test_new_inner_state_errors,
							test_state_errors,
							test_new_outer_state_errors,
							target_max_update);
					}
				}

				vector<double> test_new_state_errors;
				for (int i_index = 0; i_index < this->curr_num_new_inner_states; i_index++) {
					if (this->test_state_not_needed_locally[f_index][this->sum_inner_inputs+i_index]) {
						test_new_state_errors.push_back(0.0);
					} else {
						test_new_state_errors.push_back(test_new_inner_state_errors[this->sum_inner_inputs+i_index]);
					}
				}
				for (int n_index = (int)test_inner_state_network_histories[f_index].size()-1; n_index >= 0; n_index--) {
					for (int i_index = this->curr_num_new_inner_states-1; i_index >= 0; i_index--) {
						if (test_inner_state_network_histories[f_index][n_index][i_index] != NULL) {
							StateNetwork* state_network = test_inner_state_network_histories[f_index][n_index][i_index]->network;
							state_network->new_external_backprop(
								test_new_state_errors[i_index],
								test_new_state_errors,
								target_max_update,
								test_inner_state_network_histories[f_index][n_index][i_index]);
						}
					}
				}
				for (int i_index = 0; i_index < this->curr_num_new_inner_states; i_index++) {
					if (!this->test_state_not_needed_locally[f_index][this->sum_inner_inputs+i_index]) {
						test_new_inner_state_errors[this->sum_inner_inputs+i_index] = test_new_state_errors[i_index];
					}
				}

				for (int i_index = 0; i_index < this->num_inner_inputs[f_index]; i_index++) {
					if (this->curr_inner_inputs_needed[this->inner_input_start_indexes[f_index] + i_index]) {
						double inner_input_error = inner_input_vals_snapshots[f_index][i_index]
							- test_inner_input_vals_snapshots[f_index][i_index];
						this->sum_error += abs(inner_input_error);
						test_new_inner_state_errors[this->inner_input_start_indexes[f_index] + i_index] = inner_input_error;
						// set instead of sum as not connected
					}
				}

				for (int i_index = this->inner_input_start_indexes[f_index]+this->num_inner_inputs[f_index]-1; i_index >= 0; i_index--) {
					// state_networks_not_needed already set in correspondence with inner_inputs_needed
					if (!this->test_state_networks_not_needed[f_index][i_index]) {
						this->test_state_networks[f_index][i_index]->new_sequence_backprop(
							test_new_inner_state_errors[i_index],
							test_new_inner_state_errors,
							test_state_errors,
							test_new_outer_state_errors,
							target_max_update);
					}
				}
			} else {
				for (int s_index = this->num_sequence_states-1; s_index >= 0; s_index--) {
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
								target_max_update);
						}
					}
				}
				for (int i_index = this->sum_inner_inputs+this->curr_num_new_inner_states-1; i_index >= 0; i_index--) {
					// state_networks_not_needed already set in correspondence with inner_inputs_needed
					if (!this->test_state_networks_not_needed[f_index][i_index]) {
						this->test_state_networks[f_index][i_index]->new_sequence_backprop(
							test_new_inner_state_errors[i_index],
							test_new_inner_state_errors,
							test_state_errors,
							test_new_outer_state_errors,
							target_max_update);
					}
				}
			}
		}

		this->state_iter++;
		this->sub_iter++;
		history->state_iter_snapshot = this->state_iter;
	}
}

void Fold::clean_backprop(vector<double>& state_errors,
						  vector<bool>& states_initialized,
						  double target_val,
						  double final_misguess,
						  double final_sum_impact,
						  double& predicted_score,
						  double& scale_factor,
						  RunHelper& run_helper,
						  FoldHistory* history) {
	if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN) {
		vector<double> new_inner_state_errors(this->sum_inner_inputs+this->curr_num_new_inner_states, 0.0);
		vector<double> new_outer_state_errors(this->curr_num_new_outer_states, 0.0);

		for (int f_index = this->sequence_length-1; f_index >= 0; f_index--) {
			for (int i_index = 0; i_index < this->curr_num_states_cleared[f_index]; i_index++) {
				new_inner_state_errors[i_index] = 0.0;
			}

			this->curr_score_networks[f_index]->new_sequence_backprop_errors_with_no_weight_change(
				target_val - predicted_score,
				new_inner_state_errors,
				state_errors,
				new_outer_state_errors,
				history->score_network_histories[f_index]);

			predicted_score -= scale_factor*history->score_network_updates[f_index];

			if (this->is_inner_scope[f_index]) {
				for (int s_index = this->num_sequence_states-1; s_index >= 0; s_index--) {
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
								history->state_network_histories[f_index][state_index]);
						}
					}
				}
				for (int i_index = this->sum_inner_inputs+this->curr_num_new_inner_states-1;
						i_index >= this->inner_input_start_indexes[f_index] + this->num_inner_inputs[f_index]; i_index--) {
					// state_networks_not_needed already set in correspondence with inner_inputs_needed
					if (!this->curr_state_networks_not_needed[f_index][i_index]) {
						this->curr_state_networks[f_index][i_index]->new_sequence_backprop_errors_with_no_weight_change(
							new_inner_state_errors[i_index],
							new_inner_state_errors,
							state_errors,
							new_outer_state_errors,
							history->state_network_histories[f_index][i_index]);
					}
				}

				vector<double> new_state_errors;
				for (int i_index = 0; i_index < this->curr_num_new_inner_states; i_index++) {
					if (this->curr_state_not_needed_locally[f_index][this->sum_inner_inputs+i_index]) {
						new_state_errors.push_back(0.0);
					} else {
						new_state_errors.push_back(new_inner_state_errors[this->sum_inner_inputs+i_index]);
					}
				}
				for (int n_index = (int)history->inner_state_network_histories[f_index].size()-1; n_index >= 0; n_index--) {
					for (int i_index = this->curr_num_new_inner_states-1; i_index >= 0; i_index--) {
						if (history->inner_state_network_histories[f_index][n_index][i_index] != NULL) {
							StateNetwork* state_network = history->inner_state_network_histories[f_index][n_index][i_index]->network;
							state_network->new_external_backprop_errors_with_no_weight_change(
								new_state_errors[i_index],
								new_state_errors,
								history->inner_state_network_histories[f_index][n_index][i_index]);
						}
					}
				}
				for (int i_index = 0; i_index < this->curr_num_new_inner_states; i_index++) {
					if (!this->curr_state_not_needed_locally[f_index][this->sum_inner_inputs+i_index]) {
						new_inner_state_errors[this->sum_inner_inputs+i_index] = new_state_errors[i_index];
					}
				}

				Scope* inner_scope = solution->scopes[this->existing_scope_ids[f_index]];
				int num_input_states_diff = inner_scope->num_states - this->num_inner_inputs[f_index];

				vector<double> inner_input_errors(new_inner_state_errors.begin() + this->inner_input_start_indexes[f_index],
					new_inner_state_errors.begin() + this->inner_input_start_indexes[f_index] + this->num_inner_inputs[f_index]);
				inner_input_errors.insert(inner_input_errors.end(), num_input_states_diff, 0.0);

				vector<bool> inner_inputs_initialized(this->curr_inner_inputs_needed.begin() + this->inner_input_start_indexes[f_index],
					this->curr_inner_inputs_needed.begin() + this->inner_input_start_indexes[f_index] + this->num_inner_inputs[f_index]);
				inner_inputs_initialized.insert(inner_inputs_initialized.end(), num_input_states_diff, false);

				inner_scope->backprop(inner_input_errors,
									  inner_inputs_initialized,
									  target_val,
									  final_misguess,
									  final_sum_impact,
									  predicted_score,
									  scale_factor,
									  run_helper,
									  history->inner_scope_histories[f_index]);

				for (int i_index = 0; i_index < this->num_inner_inputs[f_index]; i_index++) {
					if (this->curr_inner_inputs_needed[this->inner_input_start_indexes[f_index] + i_index]) {
						new_inner_state_errors[this->inner_input_start_indexes[f_index] + i_index] = inner_input_errors[i_index];
					}
				}

				for (int i_index = this->inner_input_start_indexes[f_index]+this->num_inner_inputs[f_index]-1; i_index >= 0; i_index--) {
					// state_networks_not_needed already set in correspondence with inner_inputs_needed
					if (!this->curr_state_networks_not_needed[f_index][i_index]) {
						this->curr_state_networks[f_index][i_index]->new_sequence_backprop_errors_with_no_weight_change(
							new_inner_state_errors[i_index],
							new_inner_state_errors,
							state_errors,
							new_outer_state_errors,
							history->state_network_histories[f_index][i_index]);
					}
				}
			} else {
				for (int s_index = this->num_sequence_states-1; s_index >= 0; s_index--) {
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
								history->state_network_histories[f_index][state_index]);
						}
					}
				}
				for (int i_index = this->sum_inner_inputs+this->curr_num_new_inner_states-1; i_index >= 0; i_index--) {
					// state_networks_not_needed already set in correspondence with inner_inputs_needed
					if (!this->curr_state_networks_not_needed[f_index][i_index]) {
						this->curr_state_networks[f_index][i_index]->new_sequence_backprop_errors_with_no_weight_change(
							new_inner_state_errors[i_index],
							new_inner_state_errors,
							state_errors,
							new_outer_state_errors,
							history->state_network_histories[f_index][i_index]);
					}
				}
			}
		}
	} else if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE) {
		if (history->state_iter_snapshot <= this->state_iter) {
			for (int f_index = this->sequence_length-1; f_index >= 0; f_index--) {
				this->curr_score_networks[f_index]->backprop_weights_with_no_error_signal(
					target_val - predicted_score,
					0.002,
					history->score_network_histories[f_index]);

				predicted_score -= scale_factor*history->score_network_updates[f_index];

				if (this->is_inner_scope[f_index]) {
					Scope* inner_scope = solution->scopes[this->existing_scope_ids[f_index]];
					
					// unused
					vector<double> inner_input_errors;
					vector<bool> inner_inputs_initialized;

					// TODO: calc scale_factor_error
					inner_scope->backprop(inner_input_errors,
										  inner_inputs_initialized,
										  target_val,
										  final_misguess,
										  final_sum_impact,
										  predicted_score,
										  scale_factor,
										  run_helper,
										  history->inner_scope_histories[f_index]);
				}
			}

			this->curr_starting_score_network->backprop_weights_with_no_error_signal(
				target_val - predicted_score,
				0.002,
				history->starting_score_network_history);

			predicted_score -= scale_factor*history->starting_score_update;

			clean_increment();
		}
	}
}
