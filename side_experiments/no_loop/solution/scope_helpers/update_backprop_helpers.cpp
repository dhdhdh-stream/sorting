#include "scope.h"

using namespace std;

void Scope::update_backprop(double target_val,
							ScopeHistory* history) {
	if (history->exceeded_depth) {
		return;
	}

	double predicted_score = this->average_score;
	for (map<State*, StateStatus>::iterator it = history->score_state_snapshots.begin();
			it != history->score_state_snapshots.end(); it++) {
		StateNetwork* last_network = it->second.last_network;
		if (last_network == NULL) {
			// set for backprop in the following
			it->second.val *= it->first->resolved_standard_deviation;
		} else if (it->first->resolved_networks.find(last_network) == it->first->resolved_networks.end()) {
			// set for backprop in the following
			it->second.val = (it->second.val - last_network->ending_mean)
				/ last_network->ending_standard_deviation * last_network->correlation_to_end
				* it->first->resolved_standard_deviation;
		}
		predicted_score += it->first->scale->weight * it->second.val;
	}

	double error = target_val - predicted_score;

	for (map<State*, StateStatus>::iterator it = history->score_state_snapshots.begin();
			it != history->score_state_snapshots.end(); it++) {
		it->first->scale->backprop(it->second.val*error, 0.001);

		if (it->first->scale->weight < 0.02) {
			for (int n_index = 0; n_index < (int)it->first->nodes.size(); n_index++) {
				if (it->first->nodes[n_index]->type == NODE_TYPE_ACTION) {
					ActionNode* action_node = (ActionNode*)it->first->nodes[n_index];
					for (int s_index = 0; s_index < (int)action_node->score_state_defs.size(); s_index++) {
						if (action_node->score_state_defs[s_index] == it->first) {
							action_node->score_state_scope_contexts.erase(action_node->score_state_scope_contexts.begin() + s_index);
							action_node->score_state_node_contexts.erase(action_node->score_state_node_contexts.begin() + s_index);
							action_node->score_state_defs.erase(action_node->score_state_defs.begin() + s_index);
							action_node->score_state_network_indexes.erase(action_node->score_state_network_indexes.begin() + s_index);
							break;
						}
					}
				} else if (it->first->nodes[n_index]->type == NODE_TYPE_SCOPE) {
					ScopeNode* scope_node = (ScopeNode*)it->first->nodes[n_index];
					for (int s_index = 0; s_index < (int)scope_node->score_state_defs.size(); s_index++) {
						if (scope_node->score_state_defs[s_index] == it->first) {
							scope_node->score_state_scope_contexts.erase(scope_node->score_state_scope_contexts.begin() + s_index);
							scope_node->score_state_node_contexts.erase(scope_node->score_state_node_contexts.begin() + s_index);
							scope_node->score_state_obs_indexes.erase(scope_node->score_state_obs_indexes.begin() + s_index);
							scope_node->score_state_defs.erase(scope_node->score_state_defs.begin() + s_index);
							scope_node->score_state_network_indexes.erase(scope_node->score_state_network_indexes.begin() + s_index);
							break;
						}
					}
				} else {
					BranchNode* branch_node = (BranchNode*)it->first->nodes[n_index];
					for (int s_index = 0; s_index < (int)branch_node->score_state_defs.size(); s_index++) {
						if (branch_node->score_state_defs[s_index] == it->first) {
							branch_node->score_state_scope_contexts.erase(branch_node->score_state_scope_contexts.begin() + s_index);
							branch_node->score_state_node_contexts.erase(branch_node->score_state_node_contexts.begin() + s_index);
							branch_node->score_state_defs.erase(branch_node->score_state_defs.begin() + s_index);
							branch_node->score_state_network_indexes.erase(branch_node->score_state_network_indexes.begin() + s_index);
							break;
						}
					}
				}
			}

			this->score_states.erase(it->first->id);

			delete it->first;
		}
	}

	this->average_score = 0.9999*this->average_score + 0.0001*target_val;
	double curr_score_variance = (this->average_score - target_val) * (this->average_score - target_val);
	this->score_variance = 0.9999*this->score_variance + 0.0001*curr_score_variance;
	double curr_misguess = (target_val - predicted_score) * (target_val - predicted_score);
	this->average_misguess = 0.9999*this->average_misguess + 0.0001*curr_misguess;
	double curr_misguess_variance = (this->average_misguess - curr_misguess) * (this->average_misguess - curr_misguess);
	this->misguess_variance = 0.9999*this->misguess_variance + 0.0001*curr_misguess_variance;

	if (history->obs_experiment_history != NULL) {
		this->obs_experiment->backprop(history->obs_experiment_history,
									   target_val,
									   predicted_score);
	}
}
