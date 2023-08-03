#include "sequence.h"

using namespace std;

void Sequence::first_clean_pre_activate_helper(
		bool on_path,
		vector<double>& input_vals,
		RunHelper& run_helper,
		ScopeHistory* scope_history) {
	int scope_id = scope_history->scope->id;

	map<int, vector<vector<StateNetwork*>>>::iterator it = this->state_networks.find(scope_id);

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

				if (on_path
						&& i_index == (int)scope_history->node_histories.size()-1
						&& h_index == (int)scope_history->node_histories[i_index].size()-1) {
					// do nothing
				} else {
					first_clean_pre_activate_helper(false,
													input_vals,
													run_helper,
													scope_node_history->inner_scope_history);
				}
			}
		}
	}
}

void Sequence::first_clean_step_activate_helper(
		int a_index,
		vector<double>& input_vals,
		BranchExperimentHistory* branch_experiment_history,
		RunHelper& run_helper) {
	BranchExperiment* branch_experiment = this->experiment;
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
			int scope_id = this->experiment->sequences[a_index]->scopes[l_index]->id;
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

					first_clean_pre_activate_helper(false,
													input_vals,
													run_helper,
													scope_node_history->inner_scope_history);
				}
			}
		}
	}
}

void Sequence::first_clean_activate_pull(vector<double>& input_vals,
										 vector<ForwardContextLayer>& context,
										 vector<vector<double>>& previous_vals,
										 BranchExperimentHistory* branch_experiment_history,
										 RunHelper& run_helper) {
	for (int c_index = 0; c_index < (int)context.size(); c_index++) {
		first_clean_pre_activate_helper(true,
										input_vals,
										run_helper,
										context[c_index].scope_history);

		for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
			if (this->input_types[i_index] == SEQUENCE_INPUT_TYPE_LOCAL) {
				if (c_index == context.size()-1 - this->input_local_scope_depths[i_index]) {
					if (!this->input_is_new_class[i_index]) {
						/**
						 * - update input_vals immediately so new networks use original value
						 */
						double original_val_scale = this->experiment->state_iter/200000.0;
						input_vals[i_index] += original_val_scale*context[context.size()-1 - this->input_local_scope_depths[i_index]]
							.state_vals->at(this->input_local_input_indexes[i_index]);
					}
				}
			}
		}
	}

	if (branch_experiment_history != NULL) {
		for (int a_index = 0; a_index < this->step_index; a_index++) {
			first_clean_step_activate_helper(a_index,
											 input_vals,
											 branch_experiment_history,
											 run_helper);

			for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
				if (this->input_types[i_index] == SEQUENCE_INPUT_TYPE_PREVIOUS) {
					if (a_index == this->input_previous_step_index[i_index]) {
						if (!this->input_is_new_class[i_index]) {
							double original_val_scale = this->experiment->state_iter/200000.0;;
							input_vals[i_index] += original_val_scale*previous_vals[this->input_previous_step_index[i_index]][this->input_previous_input_index[i_index]];
						}
					}
				}
			}
		}
	}

	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		if (this->input_types[i_index] == SEQUENCE_INPUT_TYPE_LOCAL) {
			double original_val_scale = (200000.0-this->experiment->state_iter)/200000.0;
			input_vals[i_index] += original_val_scale*context[context.size()-1 - this->input_local_scope_depths[i_index]]
				.state_vals->at(this->input_local_input_indexes[i_index]);
		} else if (this->input_types[i_index] == SEQUENCE_INPUT_TYPE_PREVIOUS) {
			double original_val_scale = (200000.0-this->experiment->state_iter)/200000.0;
			input_vals[i_index] += original_val_scale*previous_vals[this->input_previous_step_index[i_index]][this->input_previous_input_index[i_index]];
		}
	}
}

void Sequence::first_clean_activate_reset(vector<double>& input_vals,
										  vector<ForwardContextLayer>& context,
										  vector<vector<double>>& previous_vals) {
	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		if (this->input_types[i_index] == SEQUENCE_INPUT_TYPE_LOCAL) {
			if (this->input_is_new_class[i_index]) {
				double val_diff = input_vals[i_index] - context[context.size()-1 - this->input_local_scope_depths[i_index]]
					.state_vals[this->input_local_input_indexes[i_index]];

				double set_back_scale = (200000.0-this->experiment->state_iter)/200000.0;

				context[context.size()-1 - this->input_local_scope_depths[i_index]]
					.state_vals->at(this->input_local_input_indexes[i_index]) += set_back_scale*val_diff;
			} else {
				context[context.size()-1 - this->input_local_scope_depths[i_index]]
					.state_vals->at(this->input_local_input_indexes[i_index]) = input_vals[i_index];
			}
		} else if (this->input_types[i_index] == SEQUENCE_INPUT_TYPE_PREVIOUS) {
			if (this->input_is_new_class[i_index]) {
				double val_diff = input_vals[i_index] - previous_vals[this->input_previous_step_index[i_index]][this->input_previous_input_index[i_index]];

				double set_back_scale = (200000.0-this->experiment->state_iter)/200000.0;

				previous_vals[this->input_previous_step_index[i_index]][this->input_previous_input_index[i_index]] += set_back_scale*val_diff;
			} else {
				previous_vals[this->input_previous_step_index[i_index]][this->input_previous_input_index[i_index]] = input_vals[i_index];
			}
		}
	}
}
