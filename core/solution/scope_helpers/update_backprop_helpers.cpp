#include "scope.h"

#include <queue>

#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "helpers.h"
#include "obs_experiment.h"
#include "scale.h"
#include "scope_node.h"
#include "solution.h"
#include "state.h"
#include "state_network.h"

using namespace std;

void Scope::update_backprop(double target_val,
							ScopeHistory* history) {
	if (history->exceeded_depth) {
		return;
	}

	priority_queue<pair<int, State*>> backprop_queue;

	double predicted_score = this->average_score;
	for (map<State*, StateStatus>::iterator it = history->score_state_snapshots.begin();
			it != history->score_state_snapshots.end(); it++) {
		StateNetwork* last_network = it->second.last_network;
		// last_network != NULL
		if (it->first->resolved_network_indexes.find(last_network->index) == it->first->resolved_network_indexes.end()) {
			// set for backprop in the following
			it->second.val = (it->second.val - last_network->ending_mean)
				/ last_network->ending_standard_deviation * last_network->correlation_to_end
				* it->first->resolved_standard_deviation;
		}
		predicted_score += it->first->scale->weight * it->second.val;

		backprop_queue.push({it->second.last_updated, it->first});
	}

	if (this->obs_experiment != NULL) {
		backprop_queue.push({history->test_last_updated, NULL});
	}

	while (!backprop_queue.empty()) {
		if (backprop_queue.top().second == NULL) {
			this->obs_experiment->backprop(target_val,
										   predicted_score,
										   history->test_obs_indexes,
										   history->test_obs_vals);

			if (this->obs_experiment->state == OBS_EXPERIMENT_STATE_DONE) {
				this->obs_experiment->scope_eval(this);

				delete this->obs_experiment;
				this->obs_experiment = NULL;
			}
		} else {
			double error = target_val - predicted_score;

			map<State*, StateStatus>::iterator it = history->score_state_snapshots.find(backprop_queue.top().second);

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

				solution->states.erase(it->first->id);

				delete it->first;
			}

			predicted_score -= it->first->scale->weight * it->second.val;
		}

		backprop_queue.pop();
	}

	this->average_score = 0.9999*this->average_score + 0.0001*target_val;
	double curr_score_variance = (this->average_score - target_val) * (this->average_score - target_val);
	this->score_variance = 0.9999*this->score_variance + 0.0001*curr_score_variance;
	double curr_misguess = (target_val - predicted_score) * (target_val - predicted_score);
	this->average_misguess = 0.9999*this->average_misguess + 0.0001*curr_misguess;
	double curr_misguess_variance = (this->average_misguess - curr_misguess) * (this->average_misguess - curr_misguess);
	this->misguess_variance = 0.9999*this->misguess_variance + 0.0001*curr_misguess_variance;

	if (this->obs_experiment == NULL) {
		this->obs_experiment = create_obs_experiment(history);
	}
}