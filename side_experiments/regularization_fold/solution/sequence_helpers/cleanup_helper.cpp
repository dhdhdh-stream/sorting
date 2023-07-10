#include "sequence.h"

using namespace std;

void Sequence::cleanup_outer_activate_helper(
		vector<double>& input_vals,
		RunHelper& run_helper,
		ScopeHistory* scope_history) {
	int scope_id = scope_history->scope->id;

	map<int, vector<vector<vector<StateNetwork*>>>>::iterator it = this->state_networks.find(scope_id);

	for (int i_index = 0; i_index < (int)scope_history->node_histories.size(); i_index++) {
		for (int h_index = 0; h_index < (int)scope_history->node_histories[i_index].size(); h_index++) {
			if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_ACTION) {
				int node_id = scope_history->node_histories[i_index][h_index]->node->id;

				if (it != this->state_networks.end()
						&& node_id < (int)it->second.size()
						&& it->second[node_id].size() != 0) {
					ActionNodeHistory* action_node_history = (ActionNodeHistory*)scope_history->node_histories[i_index][h_index];

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
												  // new state vals already merged
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

				cleanup_outer_activate_helper(input_vals,
											  run_helper,
											  scope_node_history->inner_scope_history);
			}
		}
	}
}

void Sequence::cleanup_experiment_activate_helper(
		vector<double>& input_vals,
		BranchExperimentHistory* branch_experiment_history,
		RunHelper& run_helper,
		SequenceHistory* history) {
	history->step_input_vals_snapshots = vector<vector<double>>(this->step_index);
	history->step_state_network_histories = vector<vector<StateNetworkHistory*>>(this->step_index);
	for (int a_index = 0; a_index < this->step_index; a_index++) {
		if (this->experiment->step_types[a_index] == EXPLORE_STEP_TYPE_ACTION) {
			history->step_input_vals_snapshots[a_index] = input_vals;
			history->step_state_network_histories[a_index] = vector<double>(this->input_init_types.size());
			for (int i_index = 0; i_index < (int)this->input_init_types.size(); i_index++) {
				if (run_helper.can_zero && rand()%5 == 0) {
					// do nothing
				} else if (this->step_state_networks[a_index][i_index] == NULL) {
					// do nothing
				} else {
					StateNetwork* network = this->step_state_networks[a_index][i_index];
					StateNetworkHistory* network_history = new StateNetworkHistory(network);
					network->new_activate(branch_experiment_history->step_obs_snapshots[a_index],
										  branch_experiment_history->step_starting_new_state_vals_snapshots[a_index],
										  // saved as new_state_vals_snapshot even though already merged
										  input_vals[i_index],
										  network_history);
					history->step_state_network_histories[a_index][i_index] = network_history;
					input_vals[i_index] += network->output->acti_vals[0];
				}
			}
		} else {
			SequenceHistory* sequence_history = branch_experiment_history->sequence_histories[a_index];
			vector<Scope*> step_scopes = sequence_history->sequence->scopes;

			for (int s_index = 0; s_index < (int)sequence_history->node_histories.size(); s_index++) {
				map<int, vector<vector<vector<StateNetwork*>>>>::iterator it = this->state_networks.find(step_scopes[s_index]);
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
													  // new state vals already merged
													  input_vals[i_index],
													  network_history);
								action_node_history->input_state_network_histories.back()[i_index] = network_history;
								input_vals[i_index] += network->output->acti_vals[0];
							}
						}
					} else {
						ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)sequence_history->node_histories[s_index][n_index];
						ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

						cleanup_outer_activate_helper(input_vals,
													  run_helper,
													  scope_node_history->inner_scope_history);
					}
				}
			}
		}
	}
}
