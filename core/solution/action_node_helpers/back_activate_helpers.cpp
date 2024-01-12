#include "action_node.h"

#include <iostream>

#include "state_network.h"
#include "state.h"
#include "state_network.h"

using namespace std;

void ActionNode::flat_vals_back_activate(vector<int>& scope_context,
										 vector<int>& node_context,
										 vector<double>& sum_vals,
										 vector<int>& counts,
										 ActionNodeHistory* history) {
	for (int h_index = 0; h_index < (int)this->obs_experiment_indexes.size(); h_index++) {
		bool matches_context = true;
		if (this->obs_experiment_scope_contexts[h_index].size() != scope_context.size()) {
			matches_context = false;
		} else {
			for (int c_index = 0; c_index < (int)this->obs_experiment_scope_contexts[h_index].size()-1; c_index++) {
				if (this->obs_experiment_scope_contexts[h_index][c_index] != scope_context[scope_context.size()-this->obs_experiment_scope_contexts[h_index].size()+c_index]
						|| this->obs_experiment_node_contexts[h_index][c_index] != node_context[scope_context.size()-this->obs_experiment_scope_contexts[h_index].size()+c_index]) {
					matches_context = false;
					break;
				}
			}
		}

		if (matches_context) {
			if (this->obs_experiment_obs_indexes[h_index] == -1) {
				sum_vals[this->obs_experiment_indexes[h_index]] += history->obs_snapshot;
			} else {
				sum_vals[this->obs_experiment_indexes[h_index]] += history->state_snapshots[this->obs_experiment_obs_indexes[h_index]];
			}
			counts[this->obs_experiment_indexes[h_index]] += 1;
		}
	}
}

void ActionNode::rnn_vals_back_activate(vector<int>& scope_context,
										vector<int>& node_context,
										vector<int>& obs_indexes,
										vector<double>& obs_vals,
										ActionNodeHistory* history) {
	for (int h_index = 0; h_index < (int)this->obs_experiment_indexes.size(); h_index++) {
		bool matches_context = true;
		if (this->obs_experiment_scope_contexts[h_index].size() != scope_context.size()) {
			matches_context = false;
		} else {
			for (int c_index = 0; c_index < (int)this->obs_experiment_scope_contexts[h_index].size()-1; c_index++) {
				if (this->obs_experiment_scope_contexts[h_index][c_index] != scope_context[scope_context.size()-this->obs_experiment_scope_contexts[h_index].size()+c_index]
						|| this->obs_experiment_node_contexts[h_index][c_index] != node_context[scope_context.size()-this->obs_experiment_scope_contexts[h_index].size()+c_index]) {
					matches_context = false;
					break;
				}
			}
		}

		if (matches_context) {
			obs_indexes.push_back(this->obs_experiment_indexes[h_index]);
			if (this->obs_experiment_obs_indexes[h_index] == -1) {
				obs_vals.push_back(history->obs_snapshot);
			} else {
				obs_vals.push_back(history->state_snapshots[this->obs_experiment_obs_indexes[h_index]]);
			}
		}
	}
}

void ActionNode::experiment_back_activate(vector<int>& scope_context,
										  vector<int>& node_context,
										  map<State*, StateStatus>& temp_state_vals,
										  ActionNodeHistory* history) {
	for (int n_index = 0; n_index < (int)this->experiment_state_defs.size(); n_index++) {
		bool matches_context = true;
		/**
		 * - check for inequality as if recursive, can lead to errors
		 *   - context size should match
		 */
		if (this->experiment_state_scope_contexts[n_index].size() != scope_context.size()) {
			matches_context = false;
		} else {
			for (int c_index = 0; c_index < (int)this->experiment_state_scope_contexts[n_index].size()-1; c_index++) {
				if (this->experiment_state_scope_contexts[n_index][c_index] != scope_context[scope_context.size()-this->experiment_state_scope_contexts[n_index].size()+c_index]
						|| this->experiment_state_node_contexts[n_index][c_index] != node_context[scope_context.size()-this->experiment_state_scope_contexts[n_index].size()+c_index]) {
					matches_context = false;
					break;
				}
			}
		}

		if (matches_context) {
			map<State*, StateStatus>::iterator it = temp_state_vals.find(this->experiment_state_defs[n_index]);
			if (it == temp_state_vals.end()) {
				it = temp_state_vals.insert({this->experiment_state_defs[n_index], StateStatus()}).first;
			}
			StateNetwork* state_network = this->experiment_state_defs[n_index]->networks[this->experiment_state_network_indexes[n_index]];
			if (this->experiment_state_obs_indexes[n_index] == -1) {
				state_network->activate(history->obs_snapshot,
										it->second);
			} else {
				state_network->activate(history->state_snapshots[this->experiment_state_obs_indexes[n_index]],
										it->second);
			}
		}
	}
}
