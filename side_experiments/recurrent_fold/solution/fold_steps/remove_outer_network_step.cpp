#include "fold.h"

#include "action_node.h"
#include "constants.h"
#include "globals.h"
#include "scope_node.h"

using namespace std;

void Fold::remove_outer_network_score_activate_helper(
		vector<double>& new_outer_state_vals,
		vector<double>& test_new_outer_state_vals,
		ScopeHistory* scope_history,
		vector<int>& curr_scope_context,
		vector<int>& curr_node_context,
		RunHelper& run_helper,
		FoldHistory* history) {
	int scope_id = scope_history->scope->id;
	curr_scope_context.push_back(scope_id);
	curr_node_context.push_back(-1);

	map<int, vector<vector<StateNetwork*>>>::iterator it = this->curr_outer_state_networks.find(scope_id);

	map<int, vector<vector<StateNetwork*>>>::iterator test_it;
	if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE || run_helper.explore_phase == EXPLORE_PHASE_NONE) {
		test_it = this->test_outer_state_networks.find(scope_id);
	}

	for (int h_index = 0; h_index < (int)scope_history->node_histories.size(); h_index++) {
		if (scope_history->node_histories[h_index]->node->type == NODE_TYPE_ACTION) {
			if (it != this->curr_outer_state_networks.end()) {
				int node_id = scope_history->node_histories[h_index]->scope_index;
				if (node_id < (int)it->second.size()
						&& it->second[node_id].size() > 0) {
					ActionNodeHistory* action_node_history = (ActionNodeHistory*)scope_history->node_histories[h_index];
					for (int s_index = 0; s_index < this->curr_num_new_outer_states; s_index++) {
						if (!this->curr_outer_state_networks_not_needed[scope_id][node_id][s_index]) {
							it->second[node_id][s_index]->new_outer_activate(
								action_node_history->obs_snapshot,
								action_node_history->ending_local_state_snapshot,
								action_node_history->ending_input_state_snapshot,
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
					int node_id = scope_history->node_histories[h_index]->scope_index;
					if (node_id < (int)test_it->second.size()
							&& test_it->second[node_id].size() > 0) {
						history->test_outer_state_network_histories.push_back(vector<StateNetworkHistory*>());
						ActionNodeHistory* action_node_history = (ActionNodeHistory*)scope_history->node_histories[h_index];
						for (int s_index = 0; s_index < this->curr_num_new_outer_states; s_index++) {
							if (!this->test_outer_state_networks_not_needed[scope_id][node_id][s_index]) {
								StateNetworkHistory* state_network_history = new StateNetworkHistory(test_it->second[node_id][s_index]);
								test_it->second[node_id][s_index]->new_outer_activate(
									action_node_history->obs_snapshot,
									action_node_history->ending_local_state_snapshot,
									action_node_history->ending_input_state_snapshot,
									test_new_outer_state_vals,
									state_network_history);
								history->test_outer_state_network_histories.back().push_back(state_network_history);
								test_new_outer_state_vals[s_index] += test_it->second[node_id][s_index]->output->acti_vals[0];
							} else {
								history->test_outer_state_network_histories.back().push_back(NULL);
							}
						}
					}
				}
			}
		} else if (scope_history->node_histories[h_index]->node->type == NODE_TYPE_INNER_SCOPE) {
			curr_node_context.back() = scope_history->node_histories[h_index]->scope_index;

			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[h_index];
			remove_outer_network_score_activate_helper(new_outer_state_vals,
													   test_new_outer_state_vals,
													   scope_node_history->inner_scope_history,
													   curr_scope_context,
													   curr_node_context,
													   run_helper,
													   history);

			curr_node_context.back() = -1;
		}
	}

	curr_scope_context.pop_back();
	curr_node_context.pop_back();
}

void Fold::remove_outer_network_score_activate(
		vector<double>& local_state_vals,
		vector<double>& input_vals,
		vector<ScopeHistory*>& context_histories,
		RunHelper& run_helper,
		FoldHistory* history) {
	vector<double> new_outer_state_vals(this->curr_num_new_outer_states, 0.0);

	vector<double> test_new_outer_state_vals;
	if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE || run_helper.explore_phase == EXPLORE_PHASE_NONE) {
		test_new_outer_state_vals = vector<double>(this->curr_num_new_outer_states, 0.0);
	}

	ScopeHistory* scope_history = context_histories[context_histories.size() - this->scope_context.size()];
	vector<int> curr_scope_context;
	vector<int> curr_node_context;
	remove_outer_network_score_activate_helper(new_outer_state_vals,
											   test_new_outer_state_vals,
											   scope_history,
											   curr_scope_context,
											   curr_node_context,
											   run_helper,
											   history);

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
		for (int n_index = (int)history->test_outer_state_network_histories.size()-1; n_index >= 0; n_index--) {
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

		this->state_iter++;
		this->sub_state_iter++;
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

void Fold::remove_outer_network_sequence_activate(
		vector<double>& local_state_vals,
		vector<double>& input_vals,
		vector<vector<double>>& flat_vals,
		double& predicted_score,
		double& scale_factor,
		double& sum_impact,
		RunHelper& run_helper,
		FoldHistory* history) {
	vector<double> new_inner_state_vals(this->sum_inner_inputs + this->curr_num_new_inner_states, 0.0);
	vector<double> new_outer_state_vals = history->new_outer_state_vals;

	if (run_helper.explore_phase == EXPLORE_PHASE_FLAT) {
		int num_total_states = this->sum_inner_inputs
			+ this->curr_num_new_inner_states
			+ this->num_sequence_local_states
			+ this->num_sequence_input_states
			+ this->curr_num_new_outer_states;
		history->state_network_histories = vector<vector<StateNetworkHistory*>>(
			this->sequence_length, vector<StateNetworkHistory*>(num_total_states, NULL));
	}
	history->inner_scope_histories = vector<ScopeHistory*>(this->sequence_length, NULL);
	history->score_network_updates = vector<double>(this->sequence_length);
	history->score_network_histories = vector<StateNetworkHistory*>(this->sequence_length, NULL);

	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		if (this->is_inner_scope[f_index]) {
			for (int i_index = 0; i_index < this->inner_input_start_indexes[f_index] + this->num_inner_inputs[f_index]; i_index++) {
				if (run_helper.explore_phase == EXPLORE_PHASE_FLAT) {
					StateNetworkHistory* state_network_history = new StateNetworkHistory(this->curr_state_networks[f_index][i_index]);
					this->curr_state_networks[f_index][i_index]->new_sequence_activate(
						new_inner_state_vals,
						local_state_vals,
						input_vals,
						new_outer_state_vals,
						state_network_history);
					history->state_network_histories[f_index][i_index] = state_network_history;
				} else {
					this->curr_state_networks[f_index][i_index]->new_sequence_activate(
						new_inner_state_vals,
						local_state_vals,
						input_vals,
						new_outer_state_vals);
				}
				new_inner_state_vals[i_index] += this->curr_state_networks[f_index][i_index]->output->acti_vals[0];
			}

			Scope* inner_scope = solution->scopes[this->existing_scope_ids[f_index]];
			vector<double> inner_input_vals(new_inner_state_vals.begin() + this->inner_input_start_indexes[f_index],
				new_inner_state_vals.begin() + this->inner_input_start_indexes[f_index] + this->num_inner_inputs[f_index]);
			int num_input_states_diff = inner_scope->num_input_states - this->num_inner_inputs[f_index];
			inner_input_vals.insert(inner_input_vals.end(), num_input_states_diff, 0.0);

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
			inner_scope->activate(inner_input_vals,
								  flat_vals,
								  predicted_score,
								  scale_factor,
								  sum_impact,
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

			// update back state so have chance to compress front after
			for (int i_index = this->inner_input_start_indexes[f_index] + this->num_inner_inputs[f_index];
					i_index < this->sum_inner_inputs + this->curr_num_new_inner_states; i_index++) {
				if (run_helper.explore_phase == EXPLORE_PHASE_FLAT) {
					StateNetworkHistory* state_network_history = new StateNetworkHistory(this->curr_state_networks[f_index][i_index]);
					this->curr_state_networks[f_index][i_index]->new_sequence_activate(
						new_inner_state_vals,
						local_state_vals,
						input_vals,
						new_outer_state_vals,
						state_network_history);
					history->state_network_histories[f_index][i_index] = state_network_history;
				} else {
					this->curr_state_networks[f_index][i_index]->new_sequence_activate(
						new_inner_state_vals,
						local_state_vals,
						input_vals,
						new_outer_state_vals);
				}
				new_inner_state_vals[i_index] += this->curr_state_networks[f_index][i_index]->output->acti_vals[0];
			}
			for (int l_index = 0; l_index < this->num_sequence_local_states; l_index++) {
				int state_index = this->sum_inner_inputs
					+ this->curr_num_new_inner_states
					+ l_index;
				if (run_helper.explore_phase == EXPLORE_PHASE_FLAT) {
					StateNetworkHistory* state_network_history = new StateNetworkHistory(this->curr_state_networks[f_index][state_index]);
					this->curr_state_networks[f_index][state_index]->new_sequence_activate(
						new_inner_state_vals,
						local_state_vals,
						input_vals,
						new_outer_state_vals,
						state_network_history);
					history->state_network_histories[f_index][state_index] = state_network_history;
				} else {
					this->curr_state_networks[f_index][state_index]->new_sequence_activate(
						new_inner_state_vals,
						local_state_vals,
						input_vals,
						new_outer_state_vals);
				}
				local_state_vals[l_index] += this->curr_state_networks[f_index][state_index]->output->acti_vals[0];
			}
			for (int i_index = 0; i_index < this->num_sequence_input_states; i_index++) {
				int state_index = this->sum_inner_inputs
					+ this->curr_num_new_inner_states
					+ this->num_sequence_local_states
					+ i_index;
				if (run_helper.explore_phase == EXPLORE_PHASE_FLAT) {
					StateNetworkHistory* state_network_history = new StateNetworkHistory(this->curr_state_networks[f_index][state_index]);
					this->curr_state_networks[f_index][state_index]->new_sequence_activate(
						new_inner_state_vals,
						local_state_vals,
						input_vals,
						new_outer_state_vals,
						state_network_history);
					history->state_network_histories[f_index][state_index] = state_network_history;
				} else {
					this->curr_state_networks[f_index][state_index]->new_sequence_activate(
						new_inner_state_vals,
						local_state_vals,
						input_vals,
						new_outer_state_vals);
				}
				input_vals[i_index] += this->curr_state_networks[f_index][state_index]->output->acti_vals[0];
			}
			for (int o_index = 0; o_index < this->curr_num_new_outer_states; o_index++) {
				int state_index = this->sum_inner_inputs
					+ this->curr_num_new_inner_states
					+ this->num_sequence_local_states
					+ this->num_sequence_input_states
					+ o_index;
				if (run_helper.explore_phase == EXPLORE_PHASE_FLAT) {
					StateNetworkHistory* state_network_history = new StateNetworkHistory(this->curr_state_networks[f_index][state_index]);
					this->curr_state_networks[f_index][state_index]->new_sequence_activate(
						new_inner_state_vals,
						local_state_vals,
						input_vals,
						new_outer_state_vals,
						state_network_history);
					history->state_network_histories[f_index][state_index] = state_network_history;
				} else {
					this->curr_state_networks[f_index][state_index]->new_sequence_activate(
						new_inner_state_vals,
						local_state_vals,
						input_vals,
						new_outer_state_vals);
				}
				new_outer_state_vals[o_index] += this->curr_state_networks[f_index][state_index]->output->acti_vals[0];
			}
		} else {
			double obs = (*flat_vals.begin())[0];

			for (int i_index = 0; i_index < this->sum_inner_inputs + this->curr_num_new_inner_states; i_index++) {
				if (run_helper.explore_phase == EXPLORE_PHASE_FLAT) {
					StateNetworkHistory* state_network_history = new StateNetworkHistory(this->curr_state_networks[f_index][i_index]);
					this->curr_state_networks[f_index][i_index]->new_sequence_activate(
						obs,
						new_inner_state_vals,
						local_state_vals,
						input_vals,
						new_outer_state_vals,
						state_network_history);
					history->state_network_histories[f_index][i_index] = state_network_history;
				} else {
					this->curr_state_networks[f_index][i_index]->new_sequence_activate(
						obs,
						new_inner_state_vals,
						local_state_vals,
						input_vals,
						new_outer_state_vals);
				}
				new_inner_state_vals[i_index] += this->curr_state_networks[f_index][i_index]->output->acti_vals[0];
			}
			for (int l_index = 0; l_index < this->num_sequence_local_states; l_index++) {
				int state_index = this->sum_inner_inputs
					+ this->curr_num_new_inner_states
					+ l_index;
				if (run_helper.explore_phase == EXPLORE_PHASE_FLAT) {
					StateNetworkHistory* state_network_history = new StateNetworkHistory(this->curr_state_networks[f_index][state_index]);
					this->curr_state_networks[f_index][state_index]->new_sequence_activate(
						obs,
						new_inner_state_vals,
						local_state_vals,
						input_vals,
						new_outer_state_vals,
						state_network_history);
					history->state_network_histories[f_index][state_index] = state_network_history;
				} else {
					this->curr_state_networks[f_index][state_index]->new_sequence_activate(
						obs,
						new_inner_state_vals,
						local_state_vals,
						input_vals,
						new_outer_state_vals);
				}
				local_state_vals[l_index] += this->curr_state_networks[f_index][state_index]->output->acti_vals[0];
			}
			for (int i_index = 0; i_index < this->num_sequence_input_states; i_index++) {
				int state_index = this->sum_inner_inputs
					+ this->curr_num_new_inner_states
					+ this->num_sequence_local_states
					+ i_index;
				if (run_helper.explore_phase == EXPLORE_PHASE_FLAT) {
					StateNetworkHistory* state_network_history = new StateNetworkHistory(this->curr_state_networks[f_index][state_index]);
					this->curr_state_networks[f_index][state_index]->new_sequence_activate(
						obs,
						new_inner_state_vals,
						local_state_vals,
						input_vals,
						new_outer_state_vals,
						state_network_history);
					history->state_network_histories[f_index][state_index] = state_network_history;
				} else {
					this->curr_state_networks[f_index][state_index]->new_sequence_activate(
						obs,
						new_inner_state_vals,
						local_state_vals,
						input_vals,
						new_outer_state_vals);
				}
				input_vals[i_index] += this->curr_state_networks[f_index][state_index]->output->acti_vals[0];
			}
			for (int o_index = 0; o_index < this->curr_num_new_outer_states; o_index++) {
				int state_index = this->sum_inner_inputs
					+ this->curr_num_new_inner_states
					+ this->num_sequence_local_states
					+ this->num_sequence_input_states
					+ o_index;
				if (run_helper.explore_phase == EXPLORE_PHASE_FLAT) {
					StateNetworkHistory* state_network_history = new StateNetworkHistory(this->curr_state_networks[f_index][state_index]);
					this->curr_state_networks[f_index][state_index]->new_sequence_activate(
						obs,
						new_inner_state_vals,
						local_state_vals,
						input_vals,
						new_outer_state_vals,
						state_network_history);
					history->state_network_histories[f_index][state_index] = state_network_history;
				} else {
					this->curr_state_networks[f_index][state_index]->new_sequence_activate(
						obs,
						new_inner_state_vals,
						local_state_vals,
						input_vals,
						new_outer_state_vals);
				}
				new_outer_state_vals[o_index] += this->curr_state_networks[f_index][state_index]->output->acti_vals[0];
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

// FOLD_STATE_REMOVE_OUTER_NETWORK backprop generic
