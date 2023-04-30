#include "fold.h"

#include <cmath>
#include <iostream>

#include "action_node.h"
#include "constants.h"
#include "globals.h"
#include "scope_node.h"

using namespace std;

void Fold::seed_outer_scope_activate_helper(vector<double>& new_outer_state_vals,
											ScopeHistory* scope_history,
											vector<vector<StateNetworkHistory*>>& outer_state_network_histories) {
	int scope_id = scope_history->scope->id;

	map<int, vector<vector<StateNetwork*>>>::iterator it = this->test_outer_state_networks.find(scope_id);
	if (it == this->test_outer_state_networks.end()) {
		it = this->test_outer_state_networks.insert({scope_id, vector<vector<StateNetwork*>>()}).first;
	}

	int size_diff = (int)scope_history->scope->nodes.size() - (int)it->second.size();
	it->second.insert(it->second.begin(), size_diff, vector<StateNetwork*>());

	for (int i_index = 0; i_index < (int)scope_history->node_histories.size(); i_index++) {
		for (int h_index = 0; h_index < (int)scope_history->node_histories[i_index].size(); h_index++) {
			if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_ACTION) {
				int node_id = scope_history->node_histories[i_index][h_index]->scope_index;
				if (it->second[node_id].size() == 0) {
					for (int s_index = 0; s_index < this->test_num_new_outer_states; s_index++) {
						it->second[node_id].push_back(new StateNetwork(
							1,
							scope_history->scope->num_states,
							0,
							this->test_num_new_outer_states,
							20));
					}
				}
				outer_state_network_histories.push_back(vector<StateNetworkHistory*>());
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)scope_history->node_histories[i_index][h_index];
				for (int s_index = 0; s_index < this->test_num_new_outer_states; s_index++) {
					StateNetworkHistory* state_network_history = new StateNetworkHistory(it->second[node_id][s_index]);
					it->second[node_id][s_index]->new_external_activate(
						action_node_history->obs_snapshot,
						action_node_history->ending_state_snapshot,
						new_outer_state_vals,
						state_network_history);
					outer_state_network_histories.back().push_back(state_network_history);
					new_outer_state_vals[s_index] += it->second[node_id][s_index]->output->acti_vals[0];
				}
			} else if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_INNER_SCOPE) {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];
				seed_outer_scope_activate_helper(new_outer_state_vals,
												 scope_node_history->inner_scope_history,
												 outer_state_network_histories);
			}
		}
	}
}

void Fold::seed_train() {
	vector<double> new_outer_state_vals(this->test_num_new_outer_states, 0.0);

	vector<vector<StateNetworkHistory*>> outer_state_network_histories;

	seed_outer_scope_activate_helper(new_outer_state_vals,
									 this->seed_outer_context_history,
									 outer_state_network_histories);

	this->test_starting_score_network->new_external_activate(
		this->seed_state_vals_snapshot,
		new_outer_state_vals);

	double seed_predicted_score = this->seed_start_predicted_score
		+ this->seed_start_scale_factor*this->test_starting_score_network->output->acti_vals[0];

	vector<double> new_outer_state_errors(this->test_num_new_outer_states, 0.0);

	double starting_score_network_target_max_update;
	if (this->state_iter <= 100000) {
		starting_score_network_target_max_update = 0.05;
	} else {
		starting_score_network_target_max_update = 0.01;
	}
	this->test_starting_score_network->new_external_backprop(
		this->seed_target_val - seed_predicted_score,
		new_outer_state_errors,
		starting_score_network_target_max_update);

	for (int n_index = (int)outer_state_network_histories.size()-1; n_index >= 0; n_index--) {
		for (int o_index = this->test_num_new_outer_states-1; o_index >= 0; o_index--) {
			StateNetwork* state_network = outer_state_network_histories[n_index][o_index]->network;
			double state_network_target_max_update;
			if (this->state_iter <= 100000) {
				state_network_target_max_update = 0.05;
			} else {
				state_network_target_max_update = 0.01;
			}
			state_network->new_external_backprop(
				new_outer_state_errors[o_index],
				new_outer_state_errors,
				state_network_target_max_update,
				outer_state_network_histories[n_index][o_index]);
		}
	}
}
