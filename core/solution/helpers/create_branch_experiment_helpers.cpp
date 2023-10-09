#include "helpers.h"

#include <iostream>
#include <random>

#include "action_node.h"
#include "branch_experiment.h"
#include "branch_node.h"
#include "globals.h"
#include "scale.h"
#include "scope.h"
#include "scope_node.h"
#include "state.h"
#include "state_network.h"

using namespace std;

void create_branch_experiment_helper(double& sum_weight,
									 vector<double>& weights,
									 vector<AbstractNode*>& possible_nodes,
									 vector<vector<int>>& possible_scope_contexts,
									 vector<vector<int>>& possible_node_contexts,
									 vector<bool>& possible_is_branch,
									 ScopeHistory* scope_history) {
	for (int i_index = 0; i_index < (int)scope_history->node_histories.size(); i_index++) {
		for (int h_index = 0; h_index < (int)scope_history->node_histories[i_index].size(); h_index++) {
			if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_ACTION) {
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)scope_history->node_histories[i_index][h_index];
				ActionNode* action_node = (ActionNode*)action_node_history->node;
				if (action_node->experiment == NULL) {
					for (int s_index = 0; s_index < (int)action_node_history->score_state_indexes.size(); s_index++) {
						State* state_def = action_node->score_state_defs[action_node_history->score_state_indexes[s_index]];
						StateStatus state_status = action_node_history->score_state_impacts[s_index];
						StateNetwork* last_network = state_status.last_network;
						if (state_def->resolved_network_indexes.find(last_network->index) == state_def->resolved_network_indexes.end()) {
							double normalized = (state_status.val - last_network->ending_mean)
								/ last_network->ending_standard_deviation * last_network->correlation_to_end
								* state_def->resolved_standard_deviation;
							double weight = abs(normalized * state_def->scale->weight);
							sum_weight += weight;
							weights.push_back(weight);
						} else {
							double weight = abs(state_status.val * state_def->scale->weight);
							sum_weight += weight;
							weights.push_back(weight);
						}

						possible_nodes.push_back(action_node);
						possible_scope_contexts.push_back(action_node->score_state_scope_contexts[action_node_history->score_state_indexes[s_index]]);
						possible_node_contexts.push_back(action_node->score_state_node_contexts[action_node_history->score_state_indexes[s_index]]);
						possible_is_branch.push_back(false);
					}
				}
			} else if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_SCOPE) {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];
				ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

				create_branch_experiment_helper(sum_weight,
												weights,
												possible_nodes,
												possible_scope_contexts,
												possible_node_contexts,
												possible_is_branch,
												scope_node_history->inner_scope_history);

				if (scope_node->experiment == NULL) {
					for (int s_index = 0; s_index < (int)scope_node_history->score_state_indexes.size(); s_index++) {
						State* state_def = scope_node->score_state_defs[scope_node_history->score_state_indexes[s_index]];
						StateStatus state_status = scope_node_history->score_state_impacts[s_index];
						StateNetwork* last_network = state_status.last_network;
						if (state_def->resolved_network_indexes.find(last_network->index) == state_def->resolved_network_indexes.end()) {
							double normalized = (state_status.val - last_network->ending_mean)
								/ last_network->ending_standard_deviation * last_network->correlation_to_end
								* state_def->resolved_standard_deviation;
							double weight = abs(normalized * state_def->scale->weight);
							sum_weight += weight;
							weights.push_back(weight);
						} else {
							double weight = abs(state_status.val * state_def->scale->weight);
							sum_weight += weight;
							weights.push_back(weight);
						}

						possible_nodes.push_back(scope_node);
						possible_scope_contexts.push_back(scope_node->score_state_scope_contexts[scope_node_history->score_state_indexes[s_index]]);
						possible_node_contexts.push_back(scope_node->score_state_node_contexts[scope_node_history->score_state_indexes[s_index]]);
						possible_is_branch.push_back(false);
					}
				}
			} else {
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)scope_history->node_histories[i_index][h_index];
				BranchNode* branch_node = (BranchNode*)branch_node_history->node;
				if (branch_node->experiment == NULL) {
					for (int s_index = 0; s_index < (int)branch_node_history->score_state_indexes.size(); s_index++) {
						State* state_def = branch_node->score_state_defs[branch_node_history->score_state_indexes[s_index]];
						StateStatus state_status = branch_node_history->score_state_impacts[s_index];
						StateNetwork* last_network = state_status.last_network;
						if (state_def->resolved_network_indexes.find(last_network->index) == state_def->resolved_network_indexes.end()) {
							double normalized = (state_status.val - last_network->ending_mean)
								/ last_network->ending_standard_deviation * last_network->correlation_to_end
								* state_def->resolved_standard_deviation;
							double weight = abs(normalized * state_def->scale->weight);
							sum_weight += weight;
							weights.push_back(weight);
						} else {
							double weight = abs(state_status.val * state_def->scale->weight);
							sum_weight += weight;
							weights.push_back(weight);
						}

						possible_nodes.push_back(branch_node);
						possible_scope_contexts.push_back(branch_node->score_state_scope_contexts[branch_node_history->score_state_indexes[s_index]]);
						possible_node_contexts.push_back(branch_node->score_state_node_contexts[branch_node_history->score_state_indexes[s_index]]);
						if (branch_node_history->obs_snapshot == 1.0) {
							possible_is_branch.push_back(false);
						} else {
							possible_is_branch.push_back(true);
						}
					}
				}
			}
		}
	}
}

void create_branch_experiment(ScopeHistory* root_history) {
	double sum_weight = 0.0;
	vector<double> weights;
	vector<AbstractNode*> possible_nodes;
	vector<vector<int>> possible_scope_contexts;
	vector<vector<int>> possible_node_contexts;
	vector<bool> possible_is_branch;

	create_branch_experiment_helper(sum_weight,
									weights,
									possible_nodes,
									possible_scope_contexts,
									possible_node_contexts,
									possible_is_branch,
									root_history);

	if (possible_nodes.size() > 0) {
		uniform_real_distribution<double> distribution(0.0, sum_weight);
		double rand_val = distribution(generator);
		for (int p_index = 0; p_index < (int)possible_nodes.size(); p_index++) {
			rand_val -= weights[p_index];
			if (rand_val <= 0.0) {
				BranchExperiment* new_branch_experiment = new BranchExperiment(
					possible_scope_contexts[p_index],
					possible_node_contexts[p_index]);

				if (possible_nodes[p_index]->type == NODE_TYPE_ACTION) {
					ActionNode* action_node = (ActionNode*)possible_nodes[p_index];
					action_node->experiment = new_branch_experiment;
				} else if (possible_nodes[p_index]->type == NODE_TYPE_SCOPE) {
					ScopeNode* scope_node = (ScopeNode*)possible_nodes[p_index];
					scope_node->experiment = new_branch_experiment;
				} else {
					BranchNode* branch_node = (BranchNode*)possible_nodes[p_index];
					branch_node->experiment = new_branch_experiment;
					branch_node->experiment_is_branch = possible_is_branch[p_index];
				}

				break;
			}
		}
	}
	/**
	 * - may fail if all nodes have branch_experiments, but context didn't match (?)
	 */
}