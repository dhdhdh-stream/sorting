#include "solution_helpers.h"

#include <algorithm>
#include <cmath>
#include <iostream>

#include "abstract_scope.h"
#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "familiarity_network.h"
#include "globals.h"
#include "info_branch_node.h"
#include "nn_helpers.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

void add_branch_node_familiarity(BranchNode* branch_node,
								 vector<AbstractScopeHistory*>& scope_histories) {
	shuffle(scope_histories.begin(), scope_histories.end(), generator);

	int num_instances = (int)scope_histories.size();

	vector<vector<double>> input_vals(num_instances);
	for (int d_index = 0; d_index < num_instances; d_index++) {
		vector<double> curr_input_vals(branch_node->input_scope_contexts.size(), 0.0);
		for (int i_index = 0; i_index < (int)branch_node->input_scope_contexts.size(); i_index++) {
			int curr_layer = 0;
			AbstractScopeHistory* curr_scope_history = scope_histories[d_index];
			while (true) {
				map<AbstractNode*, AbstractNodeHistory*>::iterator it = curr_scope_history->node_histories.find(
					branch_node->input_node_contexts[i_index][curr_layer]);
				if (it == curr_scope_history->node_histories.end()) {
					break;
				} else {
					if (curr_layer == (int)branch_node->input_scope_contexts[i_index].size()-1) {
						switch (it->first->type) {
						case NODE_TYPE_ACTION:
							{
								ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
								curr_input_vals[i_index] = action_node_history->obs_snapshot[branch_node->input_obs_indexes[i_index]];
							}
							break;
						case NODE_TYPE_BRANCH:
							{
								BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
								curr_input_vals[i_index] = branch_node_history->score;
							}
							break;
						case NODE_TYPE_INFO_BRANCH:
							{
								InfoBranchNodeHistory* info_branch_node_history = (InfoBranchNodeHistory*)it->second;
								if (info_branch_node_history->is_branch) {
									curr_input_vals[i_index] = 1.0;
								} else {
									curr_input_vals[i_index] = -1.0;
								}
							}
							break;
						}
						break;
					} else {
						curr_layer++;
						curr_scope_history = ((ScopeNodeHistory*)it->second)->scope_history;
					}
				}
			}
		}
		input_vals[d_index] = curr_input_vals;
	}

	branch_node->familiarity_networks = vector<FamiliarityNetwork*>(branch_node->input_scope_contexts.size());
	branch_node->input_means = vector<double>(branch_node->input_scope_contexts.size());
	branch_node->input_standard_deviations = vector<double>(branch_node->input_scope_contexts.size());
	branch_node->familiarity_average_misguesses = vector<double>(branch_node->input_scope_contexts.size());
	branch_node->familiarity_misguess_standard_deviations = vector<double>(branch_node->input_scope_contexts.size());
	for (int i_index = 0; i_index < (int)branch_node->input_scope_contexts.size(); i_index++) {
		double sum_val = 0.0;
		for (int d_index = 0; d_index < num_instances; d_index++) {
			sum_val += input_vals[d_index][i_index];
		}
		branch_node->input_means[i_index] = sum_val / num_instances;

		double sum_variance = 0.0;
		for (int d_index = 0; d_index < num_instances; d_index++) {
			sum_variance += (branch_node->input_means[i_index] - input_vals[d_index][i_index]) * (branch_node->input_means[i_index] - input_vals[d_index][i_index]);
		}
		branch_node->input_standard_deviations[i_index] = sqrt(sum_variance / num_instances);
		if (branch_node->input_standard_deviations[i_index] < MIN_STANDARD_DEVIATION) {
			branch_node->input_standard_deviations[i_index] = MIN_STANDARD_DEVIATION;
		}

		FamiliarityNetwork* network = new FamiliarityNetwork((int)branch_node->input_scope_contexts.size() - 1);
		branch_node->familiarity_networks[i_index] = network;

		vector<vector<double>> familiarity_vals;
		vector<double> familiarity_targets;
		for (int d_index = 0; d_index < num_instances; d_index++) {
			vector<double> curr_vals = input_vals[d_index];
			curr_vals.erase(curr_vals.begin() + i_index);
			familiarity_vals.push_back(curr_vals);

			double target_val = (input_vals[d_index][i_index] - branch_node->input_means[i_index])
				/ branch_node->input_standard_deviations[i_index];
			familiarity_targets.push_back(target_val);
		}

		train_familiarity_network(familiarity_vals,
								  familiarity_targets,
								  network);

		measure_familiarity_network(familiarity_vals,
									familiarity_targets,
									network,
									branch_node->familiarity_average_misguesses[i_index],
									branch_node->familiarity_misguess_standard_deviations[i_index]);
	}
}

void add_eval_familiarity(Scope* scope,
						  vector<AbstractScopeHistory*>& scope_histories) {
	shuffle(scope_histories.begin(), scope_histories.end(), generator);

	int num_instances = (int)scope_histories.size();

	vector<vector<double>> input_vals(num_instances);
	for (int d_index = 0; d_index < num_instances; d_index++) {
		vector<double> curr_input_vals(scope->eval_input_scope_contexts.size(), 0.0);
		for (int i_index = 0; i_index < (int)scope->eval_input_scope_contexts.size(); i_index++) {
			int curr_layer = 0;
			AbstractScopeHistory* curr_scope_history = scope_histories[d_index];
			while (true) {
				map<AbstractNode*, AbstractNodeHistory*>::iterator it = curr_scope_history->node_histories.find(
					scope->eval_input_node_contexts[i_index][curr_layer]);
				if (it == curr_scope_history->node_histories.end()) {
					break;
				} else {
					if (curr_layer == (int)scope->eval_input_scope_contexts[i_index].size()-1) {
						switch (it->first->type) {
						case NODE_TYPE_ACTION:
							{
								ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
								curr_input_vals[i_index] = action_node_history->obs_snapshot[scope->eval_input_obs_indexes[i_index]];
							}
							break;
						case NODE_TYPE_BRANCH:
							{
								BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
								curr_input_vals[i_index] = branch_node_history->score;
							}
							break;
						case NODE_TYPE_INFO_BRANCH:
							{
								InfoBranchNodeHistory* info_branch_node_history = (InfoBranchNodeHistory*)it->second;
								if (info_branch_node_history->is_branch) {
									curr_input_vals[i_index] = 1.0;
								} else {
									curr_input_vals[i_index] = -1.0;
								}
							}
							break;
						}
						break;
					} else {
						curr_layer++;
						curr_scope_history = ((ScopeNodeHistory*)it->second)->scope_history;
					}
				}
			}
		}
		input_vals[d_index] = curr_input_vals;
	}

	scope->familiarity_networks = vector<FamiliarityNetwork*>(scope->eval_input_scope_contexts.size());
	scope->input_means = vector<double>(scope->eval_input_scope_contexts.size());
	scope->input_standard_deviations = vector<double>(scope->eval_input_scope_contexts.size());
	scope->familiarity_average_misguesses = vector<double>(scope->eval_input_scope_contexts.size());
	scope->familiarity_misguess_standard_deviations = vector<double>(scope->eval_input_scope_contexts.size());
	for (int i_index = 0; i_index < (int)scope->eval_input_scope_contexts.size(); i_index++) {
		double sum_val = 0.0;
		for (int d_index = 0; d_index < num_instances; d_index++) {
			sum_val += input_vals[d_index][i_index];
		}
		scope->input_means[i_index] = sum_val / num_instances;

		double sum_variance = 0.0;
		for (int d_index = 0; d_index < num_instances; d_index++) {
			sum_variance += (scope->input_means[i_index] - input_vals[d_index][i_index]) * (scope->input_means[i_index] - input_vals[d_index][i_index]);
		}
		scope->input_standard_deviations[i_index] = sqrt(sum_variance / num_instances);
		if (scope->input_standard_deviations[i_index] < MIN_STANDARD_DEVIATION) {
			scope->input_standard_deviations[i_index] = MIN_STANDARD_DEVIATION;
		}

		FamiliarityNetwork* network = new FamiliarityNetwork((int)scope->eval_input_scope_contexts.size() - 1);
		scope->familiarity_networks[i_index] = network;

		vector<vector<double>> familiarity_vals;
		vector<double> familiarity_targets;
		for (int d_index = 0; d_index < num_instances; d_index++) {
			vector<double> curr_vals = input_vals[d_index];
			curr_vals.erase(curr_vals.begin() + i_index);
			familiarity_vals.push_back(curr_vals);

			double target_val = (input_vals[d_index][i_index] - scope->input_means[i_index])
				/ scope->input_standard_deviations[i_index];
			familiarity_targets.push_back(target_val);
		}

		train_familiarity_network(familiarity_vals,
								  familiarity_targets,
								  network);

		measure_familiarity_network(familiarity_vals,
									familiarity_targets,
									network,
									scope->familiarity_average_misguesses[i_index],
									scope->familiarity_misguess_standard_deviations[i_index]);
	}
}

void measure_familiarity(Scope* scope,
						 vector<AbstractScopeHistory*>& scope_histories) {
	vector<double> new_sum_misguesses(scope->eval_input_scope_contexts.size(), 0.0);

	int num_instances = (int)scope_histories.size();

	for (int d_index = 0; d_index < num_instances; d_index++) {
		vector<double> curr_input_vals(scope->eval_input_scope_contexts.size(), 0.0);
		for (int i_index = 0; i_index < (int)scope->eval_input_scope_contexts.size(); i_index++) {
			int curr_layer = 0;
			AbstractScopeHistory* curr_scope_history = scope_histories[d_index];
			while (true) {
				map<AbstractNode*, AbstractNodeHistory*>::iterator it = curr_scope_history->node_histories.find(
					scope->eval_input_node_contexts[i_index][curr_layer]);
				if (it == curr_scope_history->node_histories.end()) {
					break;
				} else {
					if (curr_layer == (int)scope->eval_input_scope_contexts[i_index].size()-1) {
						switch (it->first->type) {
						case NODE_TYPE_ACTION:
							{
								ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
								curr_input_vals[i_index] = action_node_history->obs_snapshot[scope->eval_input_obs_indexes[i_index]];
							}
							break;
						case NODE_TYPE_BRANCH:
							{
								BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
								curr_input_vals[i_index] = branch_node_history->score;
							}
							break;
						case NODE_TYPE_INFO_BRANCH:
							{
								InfoBranchNodeHistory* info_branch_node_history = (InfoBranchNodeHistory*)it->second;
								if (info_branch_node_history->is_branch) {
									curr_input_vals[i_index] = 1.0;
								} else {
									curr_input_vals[i_index] = -1.0;
								}
							}
							break;
						}
						break;
					} else {
						curr_layer++;
						curr_scope_history = ((ScopeNodeHistory*)it->second)->scope_history;
					}
				}
			}
		}

		for (int i_index = 0; i_index < (int)scope->eval_input_scope_contexts.size(); i_index++) {
			vector<double> curr_vals = curr_input_vals;
			curr_vals.erase(curr_vals.begin() + i_index);

			double target_val = (curr_input_vals[i_index] - scope->input_means[i_index])
				/ scope->input_standard_deviations[i_index];

			scope->familiarity_networks[i_index]->activate(curr_vals);

			double predicted_score = scope->familiarity_networks[i_index]->output->acti_vals[0];

			double misguess = (target_val - predicted_score) * (target_val - predicted_score);
			
			double normalized_misguess = abs(misguess - scope->familiarity_average_misguesses[i_index])
				/ scope->familiarity_misguess_standard_deviations[i_index];

			new_sum_misguesses[i_index] += normalized_misguess;
		}
	}

	for (int i_index = 0; i_index < (int)scope->eval_input_scope_contexts.size(); i_index++) {
		new_sum_misguesses[i_index] /= num_instances;
	}

	// temp
	for (int i_index = 0; i_index < (int)scope->eval_input_scope_contexts.size(); i_index++) {
		cout << i_index << endl;
		cout << "scope->input_means[i_index]: " << scope->input_means[i_index] << endl;
		cout << "scope->input_standard_deviations[i_index]: " << scope->input_standard_deviations[i_index] << endl;
		cout << "scope->familiarity_average_misguesses[i_index]: " << scope->familiarity_average_misguesses[i_index] << endl;
		cout << "scope->familiarity_misguess_standard_deviations[i_index]: " << scope->familiarity_misguess_standard_deviations[i_index] << endl;
		cout << "new_sum_misguesses: " << new_sum_misguesses[i_index] << endl;
		cout << endl;
	}
}
