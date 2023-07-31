#include "sequence.h"

using namespace std;

void Sequence::experiment_pre_activate_helper(
		bool on_path,
		int context_index,
		vector<double>& input_vals,
		RunHelper& run_helper,
		ScopeHistory* scope_history) {
	int scope_id = scope_history->scope->id;

	map<int, vector<vector<StateNetwork*>>>::iterator it = this->state_networks.find(scope_id);

	if (it == this->state_networks.end()) {
		it = this->state_networks.insert({scope_id, vector<vector<StateNetwork*>>()}).first;
	}
	int size_diff = (int)scope_history->scope->nodes.size() - (int)it->second.size();
	it->second.insert(it->second.end(), size_diff, vector<vector<StateNetwork*>>());

	map<int, int>::iterator seen_it = this->scope_furthest_layer_seen_in.find(scope_id);
	if (seen_it == this->scope_furthest_layer_seen_in.end()) {
		seen_it = this->scope_furthest_layer_seen_in.insert({scope_id, context_index}).first;

		// no state networks added yet
	} else {
		if (seen_it->second > context_index) {
			seen_it->second = context_index;

			int new_furthest_distance = this->experiment->scope_context.size()+2 - context_index;
			for (int n_index = 0; n_index < (int)state_it->second.size(); n_index++) {
				if (state_it->second[node_id].size() != 0) {
					for (s_index = 0; s_index < (int)this->input_init_types.size(); s_index++) {
						state_it->second[node_id][s_index]->update_lasso_weights(new_furthest_distance);
					}
				}
			}
		}
	}

	for (int i_index = 0; i_index < (int)scope_history->node_histories.size(); i_index++) {
		for (int h_index = 0; h_index < (int)scope_history->node_histories[i_index].size(); h_index++) {
			if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_ACTION) {
				int node_id = scope_history->node_histories[i_index][h_index]->node->id;

				if (it->second[node_id].size() == 0) {
					int new_furthest_distance = this->experiment->scope_context.size()+2 - seen_it->second;
					for (int ii_index = 0; ii_index < (int)this->input_init_types.size(); ii_index++) {
						it->second[node_id].push_back(
							new StateNetwork(scope_history->scope->num_states,
											 NUM_NEW_STATES,
											 1,
											 20));
						it->second[node_id].back()->update_lasso_weights(new_furthest_distance);
					}
				}

				ActionNodeHistory* action_node_history = (ActionNodeHistory*)scope_history->node_histories[i_index][h_index];

				// new_state_vals_snapshots already set

				action_node_history->experiment_sequence_step_indexes.push_back(this->step_index);
				action_node_history->input_vals_snapshots.push_back(input_vals);
				action_node_history->input_state_network_histories.push_back(vector<StateNetworkHistory*>(this->input_init_types.size(), NULL));
				for (int ii_index = 0; ii_index < (int)this->input_init_types.size(); ii_index++) {
					if (run_helper.can_zero && rand()%5 == 0) {
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
			} else if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_INNER_SCOPE) {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];
				ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

				if (on_path
						&& i_index == (int)scope_history->node_histories.size()-1
						&& h_index == (int)scope_history->node_histories[i_index].size()-1) {
					// do nothing
				} else {
					experiment_pre_activate_helper(false,
												   context_index,
												   input_vals,
												   run_helper,
												   scope_node_history->inner_scope_history);
				}
			}
		}
	}
}

void Sequence::experiment_experiment_activate_helper(
		vector<double>& input_vals,
		BranchExperimentHistory* branch_experiment_history,
		RunHelper& run_helper) {
	for (int a_index = 0; a_index < this->experiment->step_index; a_index++) {
		if (this->experiment->step_types[a_index] == BRANCH_EXPERIMENT_STEP_TYPE_ACTION) {
			branch_experiment_history->step_input_sequence_step_indexes[a_index].push_back(this->step_index);
			branch_experiment_history->step_input_vals_snapshots[a_index].push_back(input_vals);
			branch_experiment_history->step_input_state_network_histories[a_index].push_back(vector<StateNetworkHistory*>(this->input_init_types.size(), NULL));
			for (int i_index = 0; i_index < (int)this->input_init_types.size(); i_index++) {
				if (run_helper.can_zero && rand()%5 == 0) {
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
			vector<Scope*> step_scopes = sequence_history->sequence->scopes;

			for (int l_index = 0; l_index < (int)sequence_history->node_histories.size(); l_index++) {
				/**
				 * - already initialized
				 *   - furthest_layer_seen_in/steps_seen_in already set as well
				 */
				map<int, vector<vector<StateNetwork*>>>::iterator it = this->state_networks.find(step_scopes[l_index]->id);

				for (int n_index = 0; n_index < (int)sequence_history->node_histories[l_index].size(); n_index++) {
					if (sequence_history->node_histories[l_index][n_index]->node->type == NODE_TYPE_ACTION) {
						int node_id = sequence_history->node_histories[l_index][n_index]->node->id;
						ActionNodeHistory* action_node_history = (ActionNodeHistory*)sequence_history->node_histories[l_index][n_index];

						action_node_history->experiment_sequence_step_indexes.push_back(this->step_index);
						action_node_history->input_vals_snapshots.push_back(input_vals);
						action_node_history->input_state_network_histories.push_back(vector<StateNetworkHistory*>(this->input_init_types.size(), NULL));
						for (int i_index = 0; i_index < (int)this->input_init_types.size(); i_index++) {
							if (run_helper.can_zero && rand()%5 == 0) {
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
						ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)sequence_history->node_histories[l_index][n_index];
						ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

						experiment_pre_activate_helper(false,
													   this->experiment->scope_context.size()+1,
													   input_vals,
													   run_helper,
													   scope_node_history->inner_scope_history);
					}
				}
			}
		}
	}
}
