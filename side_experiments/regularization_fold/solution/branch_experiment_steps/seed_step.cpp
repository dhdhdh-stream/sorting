#include "branch_experiment.h"

using namespace std;

void BranchExperiment::seed_outer_activate_helper(int context_index,
												  vector<double>& new_state_vals,
												  bool can_zero,
												  vector<double>& obs_snapshots,
												  vector<vector<double>>& state_vals_snapshots,
												  vector<vector<double>>& new_state_vals_snapshots,
												  vector<vector<StateNetworkHistory*>>& new_state_network_histories,
												  ScopeHistory* scope_history) {
	int scope_id = scope_history->scope->id;

	map<int, vector<vector<StateNetwork*>>>::iterator state_it = this->action_node_state_networks.find(scope_id);
	map<int, vector<ScoreNetwork*>>::iterator score_it = this->action_node_score_networks.find(scope_id);

	if (state_it == this->action_node_state_networks.end()) {
		state_it = this->action_node_state_networks.insert({scope_id, vector<vector<StateNetwork*>>()}).first;
		score_it = this->action_node_score_networks.insert({scope_id, vector<ScoreNetwork*>()}).first;
	}

	int size_diff = (int)scope_history->scope->nodes.size() - (int)state_it->second.size();
	state_it->second.insert(state_it->second.end(), size_diff, vector<StateNetwork*>());
	score_it->second.insert(score_it->second.end(), size_diff, NULL);

	map<int, int>::iterator seen_it = this->scope_furthest_layer_seen_in.find(scope_id);
	if (seen_it == this->scope_furthest_layer_seen_in.end()) {
		seen_it[scope_id] = context_index;

		// no state networks added yet
	} else {
		if (seen_it->second > context_index) {
			seen_it->second = context_index;

			int new_furthest_distance = this->scope.size()+2 - context_index;
			for (int n_index = 0; n_index < (int)state_it->second.size(); n_index++) {
				if (state_it->second[node_id].size() != 0) {
					for (s_index = 0; s_index < NUM_NEW_STATES; s_index++) {
						state_it->second[node_id][s_index]->update_lasso_weights(new_furthest_distance);
					}
				}
			}
		}
	}

	for (int i_index = 0; i_index < (int)scope_history->node_histories.size(); i_index++) {
		for (int h_index = 0; h_index < (int)scope_history->node_histories[i_index].size(); h_index++) {
			if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_ACTION) {
				int node_id = scope_history->node_histories[i_index][h_index]->scope_index;

				if (state_it->second[node_id].size() == 0) {
					int new_furthest_distance = this->scope_context.size()+2 - seen_it[scope_id];
					for (int s_index = 0; s_index < NUM_NEW_STATES; s_index++) {
						state_it->second[node_id].push_back(
							new StateNetwork(scope_history->scope->num_states,
											 NUM_NEW_STATES,
											 0,
											 20));
						state_it->second[node_id].back()->update_lasso_weights(new_furthest_distance);
					}
					score_it->second[node_id] = new ScoreNetwork(scope_history->scope->num_states,
																 NUM_NEW_STATES,
																 20);
				}

				ActionNodeHistory* action_node_history = (ActionNodeHistory*)scope_history->node_histories[i_index][h_index];

				obs_snapshots.push_back(action_node_history->obs_snapshot);
				state_vals_snapshots.push_back(action_node_history->starting_state_vals_snapshot);
				new_state_vals_snapshots.push_back(new_state_vals);
				new_state_network_histories.push_back(vector<StateNetworkHistory*>(NUM_NEW_STATES, NULL));

				for (int s_index = 0; s_index < NUM_NEW_STATES; s_index++) {
					if (can_zero && rand()%5 == 0) {
						// do nothing
					} else {
						StateNetwork* network = state_it->second[node_id][s_index];
						StateNetworkHistory* network_history = new StateNetworkHistory(network);
						network->new_activate(obs_snapshots.back(),
											  state_vals_snapshots.back(),
											  new_state_vals_snapshots.back(),
											  network_history);
						new_state_network_histories[s_index] = network_history;
						new_state_vals[s_index] += network->output->acti_vals[0];
					}
				}
			} else if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_INNER_SCOPE) {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];
				ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

				if (i_index == (int)scope_history->node_histories.size()-1
						&& h_index == (int)scope_history->node_histories[i_index].size()-1) {
					// do nothing
				} else {
					seed_outer_activate_helper(context_index,
											   new_state_vals,
											   can_zero,
											   obs_snapshots,
											   state_vals_snapshots,
											   new_state_vals_snapshots,
											   new_state_network_histories,
											   scope_node_history->inner_scope_history);
				}
			}
		}
	}
}

void BranchExperiment::seed_activate() {
	bool can_zero;
	if (rand()%5 == 0) {
		can_zero = true;
	} else {
		can_zero = false;
	}
	vector<double> new_state_vals(NUM_NEW_STATES, 0.0);

	vector<double> obs_snapshots;
	vector<vector<double>> state_vals_snapshots;
	vector<vector<double>> new_state_vals_snapshots;
	vector<vector<StateNetworkHistory*>> new_state_network_histories;

	int context_size_diff = (int)context.size() - (int)this->scope_context.size();
	for (int c_index = 0; c_index < (int)context.size(); c_index++) {
		int context_index = c_index - context_size_diff;
		if (context_index < 0) {
			context_index = 0;
		}
		seed_outer_activate_helper(context_index,
								   new_state_vals,
								   can_zero,
								   obs_snapshots,
								   state_vals_snapshots,
								   new_state_vals_snapshots,
								   new_state_network_histories,
								   this->seed_outer_context_history);
	}

	this->starting_score_network->activate(this->seed_state_vals_snapshot,
										   new_state_vals);

	double seed_predicted_score = this->seed_start_predicted_score
		+ this->seed_start_scale_factor*this->starting_score_network->output->acti_vals[0];

	vector<double> new_state_errors(NUM_NEW_STATES, 0.0);

	double starting_score_network_target_max_update;
	if (this->state_iter <= 100000) {
		starting_score_network_target_max_update = 0.05;
	} else {
		starting_score_network_target_max_update = 0.01;
	}
	this->starting_score_network->new_backprop(
		this->seed_target_val - seed_predicted_score,
		new_state_errors,
		starting_score_network_target_max_update);

	for (int n_index = (int)new_state_network_histories.size()-1; n_index >= 0; n_index--) {
		for (int s_index = 0; s_index < NUM_NEW_STATES; s_index++) {
			if (new_state_network_histories[n_index][s_index] != NULL) {
				StateNetwork* network = new_state_network_histories[n_index][s_index]->network;
				double state_network_target_max_update;
				if (this->state_iter <= 100000) {
					state_network_target_max_update = 0.05;
				} else {
					state_network_target_max_update = 0.01;
				}
				network->new_backprop(new_state_errors[s_index],
									  new_state_errors,
									  state_network_target_max_update,
									  obs_snapshots[n_index],
									  state_vals_snapshots[n_index],
									  new_state_vals_snapshots[n_index],
									  new_state_network_histories[n_index][s_index]);
			}
		}
	}

	for (int n_index = 0; n_index < new_state_network_histories.size(); n_index++) {
		for (int s_index = 0; s_index < NUM_NEW_STATES; s_index++) {
			if (new_state_network_histories[n_index][s_index] != NULL) {
				delete new_state_network_histories[n_index][s_index];
			}
		}
	}
}
