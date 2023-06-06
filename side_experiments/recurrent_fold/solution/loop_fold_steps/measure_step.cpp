#include "loop_fold.h"

#include <iostream>

#include "action_node.h"
#include "constants.h"
#include "globals.h"
#include "scope_node.h"

using namespace std;

void LoopFold::measure_outer_scope_activate_helper(vector<double>& new_outer_state_vals,
												   ScopeHistory* scope_history,
												   vector<int>& curr_scope_context,
												   vector<int>& curr_node_context,
												   RunHelper& run_helper) {
	int scope_id = scope_history->scope->id;
	curr_scope_context.push_back(scope_id);
	curr_node_context.push_back(-1);

	map<int, vector<vector<StateNetwork*>>>::iterator it = this->test_outer_state_networks.find(scope_id);

	for (int i_index = 0; i_index < (int)scope_history->node_histories.size(); i_index++) {
		for (int h_index = 0; h_index < (int)scope_history->node_histories[i_index].size(); h_index++) {
			if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_ACTION) {
				if (it != this->test_outer_state_networks.end()) {
					int node_id = scope_history->node_histories[i_index][h_index]->scope_index;
					if (node_id < (int)it->second.size()
							&& it->second[node_id].size() > 0) {
						ActionNodeHistory* action_node_history = (ActionNodeHistory*)scope_history->node_histories[i_index][h_index];
						for (int s_index = 0; s_index < this->test_num_new_outer_states; s_index++) {
							it->second[node_id][s_index]->new_outer_activate(
								action_node_history->obs_snapshot,
								action_node_history->ending_local_state_snapshot,
								action_node_history->ending_input_state_snapshot,
								new_outer_state_vals);
							new_outer_state_vals[s_index] += it->second[node_id][s_index]->output->acti_vals[0];
						}
					}
				}
			} else if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_INNER_SCOPE) {
				curr_node_context.back() = scope_history->node_histories[i_index][h_index]->scope_index;

				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];
				measure_outer_scope_activate_helper(new_outer_state_vals,
													scope_node_history->inner_scope_history,
													curr_scope_context,
													curr_node_context,
													run_helper);

				curr_node_context.back() = -1;
			}
		}
	}

	curr_scope_context.pop_back();
	curr_node_context.pop_back();
}

void LoopFold::measure_inner_scope_activate_helper(vector<double>& new_state_vals,
												   ScopeHistory* scope_history,
												   vector<int>& curr_scope_context,
												   vector<int>& curr_node_context,
												   RunHelper& run_helper) {
	int scope_id = scope_history->scope->id;
	curr_scope_context.push_back(scope_id);
	curr_node_context.push_back(-1);

	map<int, vector<vector<StateNetwork*>>>::iterator it = this->test_inner_state_networks.find(scope_id);

	for (int i_index = 0; i_index < (int)scope_history->node_histories.size(); i_index++) {
		for (int h_index = 0; h_index < (int)scope_history->node_histories[i_index].size(); h_index++) {
			if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_ACTION) {
				if (it != this->test_inner_state_networks.end()) {
					int node_id = scope_history->node_histories[i_index][h_index]->scope_index;
					if (node_id < (int)it->second.size()
							&& it->second[node_id].size() > 0) {
						ActionNodeHistory* action_node_history = (ActionNodeHistory*)scope_history->node_histories[i_index][h_index];
						for (int s_index = 0; s_index < this->test_num_new_inner_states; s_index++) {
							it->second[node_id][s_index]->new_outer_activate(
								action_node_history->obs_snapshot,
								action_node_history->ending_local_state_snapshot,
								action_node_history->ending_input_state_snapshot,
								new_state_vals);
							new_state_vals[s_index] += it->second[node_id][s_index]->output->acti_vals[0];
						}
					}
				}
			} else if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_INNER_SCOPE) {
				curr_node_context.back() = scope_history->node_histories[i_index][h_index]->scope_index;

				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];
				measure_inner_scope_activate_helper(new_state_vals,
													scope_node_history->inner_scope_history,
													curr_scope_context,
													curr_node_context,
													run_helper);

				curr_node_context.back() = -1;
			}
		}
	}

	curr_scope_context.pop_back();
	curr_node_context.pop_back();
}

void LoopFold::measure_activate(vector<double>& local_state_vals,
								vector<double>& input_vals,
								vector<vector<double>>& flat_vals,
								double& predicted_score,
								double& scale_factor,
								vector<ScopeHistory*>& context_histories,
								RunHelper& run_helper,
								LoopFoldHistory* history) {
	run_helper.explore_phase = EXPLORE_PHASE_EXPERIMENT_MEASURE;

	vector<double> new_outer_state_vals(this->test_num_new_outer_states, 0.0);

	ScopeHistory* scope_history = context_histories[context_histories.size() - this->scope_context.size()];
	vector<int> curr_scope_context;
	vector<int> curr_node_context;
	measure_outer_scope_activate_helper(new_outer_state_vals,
										scope_history,
										curr_scope_context,
										curr_node_context,
										run_helper);

	vector<double> new_inner_state_vals(this->sum_inner_inputs + this->test_num_new_inner_states, 0.0);

	for (int i_index = 0; i_index < this->sum_inner_inputs + this->test_num_new_inner_states; i_index++) {
		this->test_starting_state_networks[i_index]->new_sequence_activate(
			new_inner_state_vals,
			local_state_vals,
			input_vals,
			new_outer_state_vals);

		new_inner_state_vals[i_index] += this->test_starting_state_networks[i_index]->output->acti_vals[0];
	}

	int iter_index = 0;
	while (true) {
		if (iter_index > 7) {
			// cap number of iters for now
			break;
		}

		this->test_continue_score_network->new_sequence_activate(
			new_inner_state_vals,
			local_state_vals,
			input_vals,
			new_outer_state_vals);

		this->test_halt_score_network->new_sequence_activate(
			new_inner_state_vals,
			local_state_vals,
			input_vals,
			new_outer_state_vals);

		double score_diff = scale_factor*this->test_continue_score_network->output->acti_vals[0]
			- scale_factor*this->test_halt_score_network->output->acti_vals[0];
		// fold variance not representative yet, so use existing variance for now
		double score_standard_deviation = abs(scale_factor)*sqrt(*this->existing_score_variance);
		// TODO: not sure how network gradient descent corresponds to sample size, but simply set to 2500 for now
		double score_diff_t_value = score_diff
			/ (score_standard_deviation / sqrt(2500));
		if (score_diff_t_value > 2.326) {
			// continue
			predicted_score += scale_factor*this->test_continue_score_network->output->acti_vals[0];
		} else if (score_diff_t_value < -2.326) {
			// halt
			predicted_score += scale_factor*this->test_halt_score_network->output->acti_vals[0];
			break;
		} else {
			this->test_continue_misguess_network->new_sequence_activate(
				new_inner_state_vals,
				local_state_vals,
				input_vals,
				new_outer_state_vals);

			this->test_halt_misguess_network->new_sequence_activate(
				new_inner_state_vals,
				local_state_vals,
				input_vals,
				new_outer_state_vals);

			double misguess_diff = this->test_continue_misguess_network->output->acti_vals[0]
				- this->test_halt_misguess_network->output->acti_vals[0];
			// fold variance not representative yet, so use existing variance for now
			double misguess_standard_deviation = sqrt(*this->existing_misguess_variance);
			double misguess_diff_t_value = misguess_diff
				/ (misguess_standard_deviation / sqrt(2500));
			if (misguess_diff_t_value < -2.326) {
				// continue
				predicted_score += scale_factor*this->test_continue_score_network->output->acti_vals[0];
			} else if (misguess_diff_t_value > 2.326) {
				// halt
				predicted_score += scale_factor*this->test_halt_score_network->output->acti_vals[0];
				break;
			} else {
				// continue if no strong signal either way
				predicted_score += scale_factor*this->test_continue_score_network->output->acti_vals[0];
			}
		}

		for (int f_index = 0; f_index < this->sequence_length; f_index++) {
			if (this->is_inner_scope[f_index]) {
				for (int i_index = 0; i_index < this->inner_input_start_indexes[f_index] + this->num_inner_inputs[f_index]; i_index++) {
					this->test_state_networks[f_index][i_index]->new_sequence_activate(
						new_inner_state_vals,
						local_state_vals,
						input_vals,
						new_outer_state_vals);

					new_inner_state_vals[i_index] += this->test_state_networks[f_index][i_index]->output->acti_vals[0];
				}

				Scope* inner_scope = solution->scopes[this->existing_scope_ids[f_index]];
				vector<double> inner_input_vals(new_inner_state_vals.begin() + this->inner_input_start_indexes[f_index],
					new_inner_state_vals.begin() + this->inner_input_start_indexes[f_index] + this->num_inner_inputs[f_index]);
				int num_input_states_diff = inner_scope->num_input_states - this->num_inner_inputs[f_index];
				inner_input_vals.insert(inner_input_vals.end(), num_input_states_diff, 0.0);

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
				inner_scope->activate(inner_input_vals,
									  flat_vals,
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

				for (int i_index = 0; i_index < this->num_inner_inputs[f_index]; i_index++) {
					new_inner_state_vals[this->inner_input_start_indexes[f_index] + i_index] = inner_input_vals[i_index];
				}

				vector<double> new_state_vals;
				for (int i_index = 0; i_index < this->test_num_new_inner_states; i_index++) {
					new_state_vals.push_back(new_inner_state_vals[this->sum_inner_inputs+i_index]);
				}
				measure_inner_scope_activate_helper(new_state_vals,
													scope_history,
													inner_scope_context,	// empty again
													inner_node_context,		// empty again
													run_helper);
				for (int i_index = 0; i_index < this->test_num_new_inner_states; i_index++) {
					new_inner_state_vals[this->sum_inner_inputs+i_index] = new_state_vals[i_index];
				}

				delete scope_history;

				// update back state so have chance to compress front after
				for (int i_index = this->inner_input_start_indexes[f_index] + this->num_inner_inputs[f_index];
						i_index < this->sum_inner_inputs + this->test_num_new_inner_states; i_index++) {
					this->test_state_networks[f_index][i_index]->new_sequence_activate(
						new_inner_state_vals,
						local_state_vals,
						input_vals,
						new_outer_state_vals);

					new_inner_state_vals[i_index] += this->test_state_networks[f_index][i_index]->output->acti_vals[0];
				}
				for (int l_index = 0; l_index < this->num_local_states; l_index++) {
					int state_index = this->sum_inner_inputs
						+ this->test_num_new_inner_states
						+ l_index;
					this->test_state_networks[f_index][state_index]->new_sequence_activate(
						new_inner_state_vals,
						local_state_vals,
						input_vals,
						new_outer_state_vals);

					local_state_vals[l_index] += this->test_state_networks[f_index][state_index]->output->acti_vals[0];
				}
				for (int i_index = 0; i_index < this->num_input_states; i_index++) {
					int state_index = this->sum_inner_inputs
						+ this->test_num_new_inner_states
						+ this->num_local_states
						+ i_index;
					this->test_state_networks[f_index][state_index]->new_sequence_activate(
						new_inner_state_vals,
						local_state_vals,
						input_vals,
						new_outer_state_vals);

					input_vals[i_index] += this->test_state_networks[f_index][state_index]->output->acti_vals[0];
				}
			} else {
				double obs = (*flat_vals.begin())[0];

				for (int i_index = 0; i_index < this->sum_inner_inputs + this->test_num_new_inner_states; i_index++) {
					this->test_state_networks[f_index][i_index]->new_sequence_activate(
						obs,
						new_inner_state_vals,
						local_state_vals,
						input_vals,
						new_outer_state_vals);

					new_inner_state_vals[i_index] += this->test_state_networks[f_index][i_index]->output->acti_vals[0];
				}
				for (int l_index = 0; l_index < this->num_local_states; l_index++) {
					int state_index = this->sum_inner_inputs
						+ this->test_num_new_inner_states
						+ l_index;
					this->test_state_networks[f_index][state_index]->new_sequence_activate(
						obs,
						new_inner_state_vals,
						local_state_vals,
						input_vals,
						new_outer_state_vals);

					local_state_vals[l_index] += this->test_state_networks[f_index][state_index]->output->acti_vals[0];
				}
				for (int i_index = 0; i_index < this->num_input_states; i_index++) {
					int state_index = this->sum_inner_inputs
						+ this->test_num_new_inner_states
						+ this->num_local_states
						+ i_index;
					this->test_state_networks[f_index][state_index]->new_sequence_activate(
						obs,
						new_inner_state_vals,
						local_state_vals,
						input_vals,
						new_outer_state_vals);

					input_vals[i_index] += this->test_state_networks[f_index][state_index]->output->acti_vals[0];
				}

				flat_vals.erase(flat_vals.begin());
			}

			this->test_score_networks[f_index]->new_sequence_activate(
				new_inner_state_vals,
				local_state_vals,
				input_vals,
				new_outer_state_vals);

			predicted_score += scale_factor*this->test_score_networks[f_index]->output->acti_vals[0];
		}

		iter_index++;
	}
}

void LoopFold::measure_backprop(vector<double>& local_state_errors,
								vector<double>& input_errors,
								double target_val,
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
}