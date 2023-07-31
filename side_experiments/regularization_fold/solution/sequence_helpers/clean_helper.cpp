#include "sequence.h"

using namespace std;

void Sequence::clean_pre_activate_helper(
		bool on_path,
		vector<int>& temp_scope_context,
		vector<int>& temp_node_context,
		vector<double>& input_vals,
		RunHelper& run_helper,
		ScopeHistory* scope_history) {
	int scope_id = scope_history->scope->id;

	map<int, vector<vector<StateNetwork*>>>::iterator it = this->state_networks.find(scope_id);

	if (this->experiment->state == BRANCH_EXPERIMENT_STATE_SECOND_CLEAN) {
		for (int ii_index = 0; ii_index < (int)this->input_init_types.size(); ii_index++) {
			set<int>::iterator needed_it = this->scope_additions_needed[ii_index].find(scope_id);
			if (needed_it != this->scope_additions_needed[ii_index].end()) {
				// if needed, then starting must be on path
				int starting_index;
				if (this->input_furthest_layer_needed_in[i_index] == 0) {
					starting_index = 0;
				} else {
					starting_index = run_helper.experiment_context_start_index
						+ this->input_furthest_layer_needed_in[i_index]-1;
					// input_furthest_layer_needed_in[i_index] < scope_context.size()+2
				}
				for (int c_index = starting_index; c_index < (int)temp_scope_context.size(); c_index++) {
					this->scope_node_additions_needed[ii_index].insert({temp_scope_context[c_index], temp_node_context[c_index]});
				}
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
					action_node_history->input_state_network_histories.push_back(vector<StateNetworkHistory*>(this->input_init_types.size(), NULL));
					for (int ii_index = 0; ii_index < (int)this->input_init_types.size(); ii_index++) {
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
			} else if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_INNER_SCOPE) {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];
				ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

				temp_scope_context.push_back(scope_id);
				temp_node_context.push_back(scope_node->id);

				if (on_path
						&& i_index == (int)scope_history->node_histories.size()-1
						&& h_index == (int)scope_history->node_histories[i_index].size()-1) {
					// do nothing
				} else {
					clean_pre_activate_helper(false,
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

void Sequence::clean_experiment_activate_helper(
		vector<int>& temp_scope_context,
		vector<int>& temp_node_context,
		vector<double>& input_vals,
		BranchExperimentHistory* branch_experiment_history,
		RunHelper& run_helper) {
	for (int a_index = 0; a_index < this->step_index; a_index++) {
		if (this->experiment->step_types[a_index] == BRANCH_EXPERIMENT_STEP_TYPE_ACTION) {
			branch_experiment_history->step_input_sequence_step_indexes[a_index].push_back(this->step_index);
			branch_experiment_history->step_input_vals_snapshots[a_index].push_back(input_vals);
			branch_experiment_history->step_input_state_network_histories[a_index].push_back(vector<StateNetworkHistory*>(this->input_init_types.size(), NULL));
			for (int i_index = 0; i_index < (int)this->input_init_types.size(); i_index++) {
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

			vector<int> temp_scope_context_copy = temp_scope_context;
			vector<int> temp_node_context_copy = temp_node_context;
			// copy so don't have to reset for each sequence

			for (int s_index = 0; s_index < (int)sequence_history->node_histories.size(); s_index++) {
				int scope_id = this->experiment->sequences[a_index]->scopes[s_index]->id;
				map<int, vector<vector<StateNetwork*>>>::iterator it = this->state_networks.find(scope_id);

				for (int n_index = 0; n_index < (int)sequence_history->node_histories[s_index].size(); n_index++) {
					if (sequence_history->node_histories[s_index][n_index]->node->type == NODE_TYPE_ACTION) {
						int node_id = sequence_history->node_histories[s_index][n_index]->node->id;
						ActionNodeHistory* action_node_history = (ActionNodeHistory*)sequence_history->node_histories[s_index][n_index];

						action_node_history->experiment_sequence_step_indexes.push_back(this->step_index);
						action_node_history->input_vals_snapshots.push_back(input_vals);
						action_node_history->input_state_network_histories.push_back(vector<StateNetworkHistory*>(this->input_init_types.size(), NULL));
						for (int i_index = 0; i_index < (int)this->input_init_types.size(); i_index++) {
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
					} else {
						ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)sequence_history->node_histories[s_index][n_index];
						ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

						temp_scope_context.push_back(scope_id);
						temp_node_context.push_back(scope_node->id);

						clean_pre_activate_helper(false,
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
				temp_node_context.push_back(this->experiment->sequences[a_index]->node_ids[s_index].back());
				// for last layer, node isn't scope node, but won't matter
			}
		}
	}
}
