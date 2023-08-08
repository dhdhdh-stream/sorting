#include "sequence.h"

#include "abstract_experiment.h"
#include "abstract_node.h"
#include "action_node.h"
#include "branch_experiment.h"
#include "layer.h"
#include "scope.h"
#include "scope_node.h"
#include "state_network.h"

using namespace std;

void Sequence::second_clean_pre_activate_helper(
		bool on_path,
		vector<int>& temp_scope_context,
		vector<int>& temp_node_context,
		vector<double>& input_vals,
		RunHelper& run_helper,
		ScopeHistory* scope_history) {
	int scope_id = scope_history->scope->id;

	map<int, vector<vector<StateNetwork*>>>::iterator it = this->state_networks.find(scope_id);

	for (int ii_index = 0; ii_index < (int)this->input_types.size(); ii_index++) {
		set<int>::iterator needed_it = this->scope_additions_needed[ii_index].find(scope_id);
		if (needed_it != this->scope_additions_needed[ii_index].end()) {
			// if needed, then starting must be on path
			int starting_index;
			if (this->input_furthest_layer_needed_in[ii_index] == 0) {
				starting_index = 0;
			} else {
				starting_index = run_helper.experiment_context_start_index
					+ this->input_furthest_layer_needed_in[ii_index]-1;
				// input_furthest_layer_needed_in[ii_index] < scope_context.size()+2
			}
			for (int c_index = starting_index; c_index < (int)temp_scope_context.size(); c_index++) {
				this->scope_node_additions_needed[ii_index].insert({temp_scope_context[c_index], temp_node_context[c_index]});
			}
		}
	}

	for (int i_index = 0; i_index < (int)scope_history->node_histories.size(); i_index++) {
		for (int h_index = 0; h_index < (int)scope_history->node_histories[i_index].size(); h_index++) {
			if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_ACTION) {
				int node_id = scope_history->node_histories[i_index][h_index]->node->id;

				if (it != this->state_networks.end()
						&& node_id < (int)it->second.size()
						&& it->second[node_id].size() != 0) {
					ActionNodeHistory* action_node_history = (ActionNodeHistory*)scope_history->node_histories[i_index][h_index];

					// new_state_vals_snapshots already set

					action_node_history->experiment_sequence_step_indexes.push_back(this->step_index);
					action_node_history->input_vals_snapshots.push_back(input_vals);
					action_node_history->input_state_network_histories.push_back(vector<StateNetworkHistory*>(this->input_types.size(), NULL));
					for (int ii_index = 0; ii_index < (int)this->input_types.size(); ii_index++) {
						if (run_helper.can_zero && rand()%5 == 0) {
							// do nothing
						} else if (it->second[node_id][ii_index] == NULL) {
							// do nothing
						} else {
							StateNetwork* network = it->second[node_id][ii_index];
							StateNetworkHistory* network_history = new StateNetworkHistory(network);
							network->new_activate(action_node_history->obs_snapshot,
												  action_node_history->starting_state_vals_snapshot,
												  action_node_history->starting_new_state_vals_snapshot,
												  input_vals[ii_index],
												  network_history);
							action_node_history->input_state_network_histories.back()[ii_index] = network_history;
							input_vals[ii_index] += network->output->acti_vals[0];
						}
					}
				}
			} else if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_SCOPE) {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];
				ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

				temp_scope_context.push_back(scope_id);
				temp_node_context.push_back(scope_node->id);

				if (on_path
						&& i_index == (int)scope_history->node_histories.size()-1
						&& h_index == (int)scope_history->node_histories[i_index].size()-1) {
					// do nothing

					/**
					 * - for last layer, last node won't be ScopeNode, so nothing will be added to temp_scope_context/temp_node_context
					 */
				} else {
					second_clean_pre_activate_helper(false,
													 temp_scope_context,
													 temp_node_context,
													 input_vals,
													 run_helper,
													 scope_node_history->inner_scope_history);

					temp_scope_context.pop_back();
					temp_node_context.pop_back();
				}
			}
		}
	}
}

void Sequence::second_clean_step_activate_helper(
		int a_index,
		vector<int> temp_scope_context,
		vector<int> temp_node_context,
		// pass-by-value (i.e., copy) so don't have to reset for each sequence
		vector<double>& input_vals,
		BranchExperimentHistory* branch_experiment_history,
		RunHelper& run_helper) {
	BranchExperiment* branch_experiment = (BranchExperiment*)this->experiment;
	if (branch_experiment->step_types[a_index] == BRANCH_EXPERIMENT_STEP_TYPE_ACTION) {
		branch_experiment_history->step_input_sequence_step_indexes[a_index].push_back(this->step_index);
		branch_experiment_history->step_input_vals_snapshots[a_index].push_back(input_vals);
		branch_experiment_history->step_input_state_network_histories[a_index].push_back(vector<StateNetworkHistory*>(this->input_types.size(), NULL));
		for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
			if (run_helper.can_zero && rand()%5 == 0) {
				// do nothing
			} else if (this->step_state_networks[a_index][i_index] == NULL) {
				// do nothing
			} else {
				StateNetwork* network = this->step_state_networks[a_index][i_index];
				vector<double> empty_state_vals;
				StateNetworkHistory* network_history = new StateNetworkHistory(network);
				network->new_activate(branch_experiment_history->step_obs_snapshots[a_index],
									  empty_state_vals,
									  branch_experiment_history->step_starting_new_state_vals_snapshots[a_index],
									  input_vals[i_index],
									  network_history);
				branch_experiment_history->step_input_state_network_histories[a_index].back()[i_index] = network_history;
				input_vals[i_index] += network->output->acti_vals[0];
			}
		}
	} else {
		SequenceHistory* sequence_history = branch_experiment_history->sequence_histories[a_index];

		for (int l_index = 0; l_index < (int)sequence_history->node_histories.size(); l_index++) {
			int scope_id = branch_experiment->sequences[a_index]->scopes[l_index]->id;
			map<int, vector<vector<StateNetwork*>>>::iterator it = this->state_networks.find(scope_id);

			for (int n_index = 0; n_index < (int)sequence_history->node_histories[l_index].size(); n_index++) {
				if (sequence_history->node_histories[l_index][n_index]->node->type == NODE_TYPE_ACTION) {
					int node_id = sequence_history->node_histories[l_index][n_index]->node->id;
					ActionNodeHistory* action_node_history = (ActionNodeHistory*)sequence_history->node_histories[l_index][n_index];

					action_node_history->experiment_sequence_step_indexes.push_back(this->step_index);
					action_node_history->input_vals_snapshots.push_back(input_vals);
					action_node_history->input_state_network_histories.push_back(vector<StateNetworkHistory*>(this->input_types.size(), NULL));
					for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
						if (run_helper.can_zero && rand()%5 == 0) {
							// do nothing
						} else if (it->second[node_id][i_index] == NULL) {
							// do nothing
						} else {
							StateNetwork* network = it->second[node_id][i_index];
							StateNetworkHistory* network_history = new StateNetworkHistory(network);
							network->new_activate(action_node_history->obs_snapshot,
												  action_node_history->starting_state_vals_snapshot,
												  action_node_history->starting_new_state_vals_snapshot,
												  input_vals[i_index],
												  network_history);
							action_node_history->input_state_network_histories.back()[i_index] = network_history;
							input_vals[i_index] += network->output->acti_vals[0];
						}
					}
				} else if (sequence_history->node_histories[l_index][n_index]->node->type == NODE_TYPE_SCOPE) {
					ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)sequence_history->node_histories[l_index][n_index];
					ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

					temp_scope_context.push_back(scope_id);
					temp_node_context.push_back(scope_node->id);

					second_clean_pre_activate_helper(false,
													 temp_scope_context,
													 temp_node_context,
													 input_vals,
													 run_helper,
													 scope_node_history->inner_scope_history);

					temp_scope_context.pop_back();
					temp_node_context.pop_back();
				}
			}

			temp_scope_context.push_back(scope_id);
			temp_node_context.push_back(branch_experiment->sequences[a_index]->node_ids[l_index].back());
			// for last layer, node isn't scope node, but won't matter
		}
	}
}

void Sequence::second_clean_activate_pull(vector<double>& input_vals,
										  vector<ForwardContextLayer>& context,
										  vector<vector<double>>& previous_vals,
										  BranchExperimentHistory* branch_experiment_history,
										  RunHelper& run_helper) {
	int context_size_diff = (int)context.size() - (int)this->experiment->scope_context.size() - 1;
	vector<int> temp_scope_context;
	vector<int> temp_node_context;
	for (int c_index = 0; c_index < (int)context.size(); c_index++) {
		second_clean_pre_activate_helper(true,
										 temp_scope_context,
										 temp_node_context,
										 input_vals,
										 run_helper,
										 context[c_index].scope_history);

		for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
			if (this->input_types[i_index] == SEQUENCE_INPUT_TYPE_LOCAL) {
				if (c_index == (int)context.size()-1 - this->input_local_scope_depths[i_index]) {
					if (!this->input_is_new_class[i_index]) {
						input_vals[i_index] += context[context.size()-1 - this->input_local_scope_depths[i_index]]
							.state_vals->at(this->input_local_input_indexes[i_index]);
					}
				}
			}
		}

		for (int cc_index = 0; cc_index < (int)this->experiment->corr_calc_scope_depths.size(); cc_index++) {
			if (c_index == (int)context.size()-1 - this->experiment->corr_calc_scope_depths[cc_index]) {
				double curr_val = context[context.size()-1 - this->experiment->corr_calc_scope_depths[cc_index]].state_vals->at(this->experiment->corr_calc_input_indexes[cc_index]);
				for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
					if (this->input_types[i_index] != SEQUENCE_INPUT_TYPE_LOCAL
							|| this->input_is_new_class[i_index]) {
						if (this->input_furthest_layer_needed_in[i_index] <= c_index - context_size_diff) {
							this->corr_calc_new_average_vals[cc_index][i_index] = 0.9999*this->corr_calc_new_average_vals[cc_index][i_index] + 0.0001*input_vals[i_index];
							double curr_new_variance = (this->corr_calc_new_average_vals[cc_index][i_index] - input_vals[i_index])*(this->corr_calc_new_average_vals[cc_index][i_index] - input_vals[i_index]);
							this->corr_calc_new_variances[cc_index][i_index] = 0.9999*this->corr_calc_new_variances[cc_index][i_index] + 0.0001*curr_new_variance;
							double curr_covariance = (this->experiment->corr_calc_average_vals[cc_index] - curr_val)*(this->corr_calc_new_average_vals[cc_index][i_index] - input_vals[i_index]);
							this->corr_calc_covariances[cc_index][i_index] = 0.9999*this->corr_calc_covariances[cc_index][i_index] + 0.0001*curr_covariance;
							this->new_transformations[cc_index][i_index].backprop(curr_val, input_vals[i_index]);
						}
					}
					/**
					 * - don't calculate correlation for non-new class
					 *   - should have already been calculated in previous experiments
					 */
				}
			}
		}
	}

	if (branch_experiment_history != NULL) {
		for (int a_index = 0; a_index < this->step_index; a_index++) {
			second_clean_step_activate_helper(a_index,
											  temp_scope_context,
											  temp_node_context,
											  input_vals,
											  branch_experiment_history,
											  run_helper);

			for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
				if (this->input_types[i_index] == SEQUENCE_INPUT_TYPE_PREVIOUS) {
					if (a_index == this->input_previous_step_index[i_index]) {
						if (!this->input_is_new_class[i_index]) {
							input_vals[i_index] += previous_vals[this->input_previous_step_index[i_index]][this->input_previous_input_index[i_index]];
						}
					}
				}
			}
		}
	}

	for (int s_index = 0; s_index < NUM_NEW_STATES; s_index++) {
		this->corr_calc_state_average_vals[s_index] = 0.9999*this->corr_calc_state_average_vals[s_index] + 0.0001*run_helper.new_state_vals[s_index];
		double curr_state_variance = (this->corr_calc_state_average_vals[s_index] - run_helper.new_state_vals[s_index])*(this->corr_calc_state_average_vals[s_index] - run_helper.new_state_vals[s_index]);
		this->corr_calc_state_variances[s_index] = 0.9999*this->corr_calc_state_variances[s_index] + 0.0001*curr_state_variance;

		for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
			if (this->input_types[i_index] != SEQUENCE_INPUT_TYPE_LOCAL
					|| this->input_is_new_class[i_index]) {
				this->corr_calc_input_average_vals[s_index][i_index] = 0.9999*this->corr_calc_input_average_vals[s_index][i_index] + 0.0001*input_vals[i_index];
				double curr_state_variance = (this->corr_calc_input_average_vals[s_index][i_index] - input_vals[i_index])*(this->corr_calc_input_average_vals[s_index][i_index] - input_vals[i_index]);
				this->corr_calc_input_variances[s_index][i_index] = 0.9999*this->corr_calc_input_variances[s_index][i_index] + 0.0001*curr_state_variance;
				double curr_covariance = (this->corr_calc_state_average_vals[s_index] - run_helper.new_state_vals[s_index])*(this->corr_calc_input_average_vals[s_index][i_index] - input_vals[i_index]);
				this->corr_calc_new_covariances[s_index][i_index] = 0.9999*this->corr_calc_new_covariances[s_index][i_index] + 0.0001*curr_covariance;
				this->new_new_transformations[s_index][i_index].backprop(run_helper.new_state_vals[s_index], input_vals[i_index]);
			}
		}
		/**
		 * - don't calculate correlation for non-new class
		 *   - already calculated in BranchExperiment
		 */
	}
}

void Sequence::second_clean_activate_reset(vector<double>& input_vals,
										   vector<ForwardContextLayer>& context,
										   vector<vector<double>>& previous_vals) {
	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		if (this->input_types[i_index] == SEQUENCE_INPUT_TYPE_LOCAL) {
			if (!this->input_is_new_class[i_index]) {
				context[context.size()-1 - this->input_local_scope_depths[i_index]]
					.state_vals->at(this->input_local_input_indexes[i_index]) = input_vals[i_index];
			}
		} else if (this->input_types[i_index] == SEQUENCE_INPUT_TYPE_PREVIOUS) {
			if (!this->input_is_new_class[i_index]) {
				previous_vals[this->input_previous_step_index[i_index]][this->input_previous_input_index[i_index]] = input_vals[i_index];
			}
		}
	}
}

void Sequence::second_clean_backprop_pull(vector<double>& input_errors,
										  vector<BackwardContextLayer>& context,
										  vector<vector<double>>& previous_errors) {
	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		if (this->input_types[i_index] == SEQUENCE_INPUT_TYPE_LOCAL) {
			if (!this->input_is_new_class[i_index]) {
				input_errors[i_index] = context[context.size()-1 - this->input_local_scope_depths[i_index]]
					.state_errors->at(this->input_local_input_indexes[i_index]);
			}
		} else if (this->input_types[i_index] == SEQUENCE_INPUT_TYPE_PREVIOUS) {
			if (!this->input_is_new_class[i_index]) {
				input_errors[i_index] = previous_errors[this->input_previous_step_index[i_index]][this->input_previous_input_index[i_index]];
			}
		}
	}
}

void Sequence::second_clean_backprop_reset(vector<double>& input_errors,
										   vector<BackwardContextLayer>& context,
										   vector<vector<double>>& previous_errors) {
	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		if (this->input_types[i_index] == SEQUENCE_INPUT_TYPE_LOCAL) {
			if (!this->input_is_new_class[i_index]) {
				context[context.size()-1 - this->input_local_scope_depths[i_index]]
					.state_errors->at(this->input_local_input_indexes[i_index]) = input_errors[i_index];
			}
		} else if (this->input_types[i_index] == SEQUENCE_INPUT_TYPE_PREVIOUS) {
			if (!this->input_is_new_class[i_index]) {
				previous_errors[this->input_previous_step_index[i_index]][this->input_previous_input_index[i_index]] = input_errors[i_index];
			}
		}
	}
}
