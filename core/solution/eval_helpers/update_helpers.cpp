#include "eval_helpers.h"

#include <cmath>
#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_scope_node.h"
#include "network.h"
#include "nn_helpers.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_helpers.h"

using namespace std;

void update_eval(Scope* parent_scope,
				 vector<ScopeHistory*>& scope_histories,
				 vector<double>& target_val_histories) {
	int num_instances = (int)scope_histories.size();

	vector<vector<double>> inputs(num_instances);
	double average_misguess;
	double misguess_standard_deviation;
	if (parent_scope->eval_network != NULL) {
		for (int d_index = 0; d_index < num_instances; d_index++) {
			vector<double> d_inputs(parent_scope->eval_input_node_contexts.size(), 0.0);
			for (int i_index = 0; i_index < (int)parent_scope->eval_input_node_contexts.size(); i_index++) {
				int curr_layer = 0;
				ScopeHistory* curr_scope_history = scope_histories[d_index];
				while (true) {
					map<AbstractNode*, AbstractNodeHistory*>::iterator it = curr_scope_history->node_histories.find(
						parent_scope->eval_input_node_contexts[i_index][curr_layer]);
					if (it == curr_scope_history->node_histories.end()) {
						break;
					} else {
						if (curr_layer == (int)parent_scope->eval_input_scope_contexts[i_index].size()-1) {
							switch (it->first->type) {
							case NODE_TYPE_ACTION:
								{
									ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
									d_inputs[i_index] = action_node_history->obs_snapshot[parent_scope->eval_input_obs_indexes[i_index]];
								}
								break;
							case NODE_TYPE_BRANCH:
								{
									BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
									if (branch_node_history->is_branch) {
										d_inputs[i_index] = 1.0;
									} else {
										d_inputs[i_index] = -1.0;
									}
								}
								break;
							case NODE_TYPE_INFO_SCOPE:
								{
									InfoScopeNodeHistory* info_scope_node_history = (InfoScopeNodeHistory*)it->second;
									if (info_scope_node_history->is_positive) {
										d_inputs[i_index] = 1.0;
									} else {
										d_inputs[i_index] = -1.0;
									}
								}
								break;
							case NODE_TYPE_INFO_BRANCH:
								{
									InfoBranchNodeHistory* info_branch_node_history = (InfoBranchNodeHistory*)it->second;
									if (info_branch_node_history->is_branch) {
										d_inputs[i_index] = 1.0;
									} else {
										d_inputs[i_index] = -1.0;
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
			inputs[d_index] = d_inputs;
		}

		optimize_w_drop_network(inputs,
								target_val_histories,
								parent_scope->eval_network);

		measure_network(inputs,
						target_val_histories,
						parent_scope->eval_network,
						average_misguess,
						misguess_standard_deviation);
	}

	int train_index = 0;
	while (train_index < 3) {
		vector<vector<Scope*>> test_input_scope_contexts = parent_scope->eval_input_scope_contexts;
		vector<vector<AbstractNode*>> test_input_node_contexts = parent_scope->eval_input_node_contexts;
		vector<int> test_input_obs_indexes = parent_scope->eval_input_obs_indexes;

		vector<vector<Scope*>> possible_scope_contexts;
		vector<vector<AbstractNode*>> possible_node_contexts;
		vector<int> possible_obs_indexes;

		vector<Scope*> scope_context;
		vector<AbstractNode*> node_context;
		uniform_int_distribution<int> history_distribution(0, num_instances-1);
		gather_possible_helper(scope_context,
							   node_context,
							   possible_scope_contexts,
							   possible_node_contexts,
							   possible_obs_indexes,
							   scope_histories[history_distribution(generator)]);

		vector<int> remaining_indexes(possible_node_contexts.size());
		for (int p_index = 0; p_index < (int)possible_node_contexts.size(); p_index++) {
			remaining_indexes[p_index] = p_index;
		}
		int num_new_input = 0;
		while (true) {
			uniform_int_distribution<int> distribution(0, (int)remaining_indexes.size()-1);
			int rand_index = distribution(generator);

			bool contains = false;
			for (int i_index = 0; i_index < (int)test_input_node_contexts.size(); i_index++) {
				if (possible_scope_contexts[remaining_indexes[rand_index]] == test_input_scope_contexts[i_index]
						&& possible_node_contexts[remaining_indexes[rand_index]] == test_input_node_contexts[i_index]
						&& possible_obs_indexes[remaining_indexes[rand_index]] == test_input_obs_indexes[i_index]) {
					contains = true;
					break;
				}
			}
			if (!contains) {
				test_input_scope_contexts.push_back(possible_scope_contexts[remaining_indexes[rand_index]]);
				test_input_node_contexts.push_back(possible_node_contexts[remaining_indexes[rand_index]]);
				test_input_obs_indexes.push_back(possible_obs_indexes[remaining_indexes[rand_index]]);
				num_new_input++;
			}

			remaining_indexes.erase(remaining_indexes.begin() + rand_index);

			if (num_new_input >= NETWORK_INCREMENT_NUM_NEW
					|| remaining_indexes.size() == 0) {
				break;
			}
		}

		Network* test_network;
		if (parent_scope->eval_network == NULL) {
			test_network = new Network();
		} else {
			test_network = new Network(parent_scope->eval_network);
		}
		test_network->increment((int)test_input_node_contexts.size());

		vector<vector<double>> test_inputs = inputs;
		for (int d_index = 0; d_index < num_instances; d_index++) {
			test_inputs[d_index].reserve((int)test_input_node_contexts.size());
			for (int t_index = (int)parent_scope->eval_input_node_contexts.size(); t_index < (int)test_input_node_contexts.size(); t_index++) {
				int curr_layer = 0;
				ScopeHistory* curr_scope_history = scope_histories[d_index];
				while (true) {
					map<AbstractNode*, AbstractNodeHistory*>::iterator it = curr_scope_history->node_histories.find(
						test_input_node_contexts[t_index][curr_layer]);
					if (it == curr_scope_history->node_histories.end()) {
						test_inputs[d_index].push_back(0.0);
						break;
					} else {
						if (curr_layer == (int)test_input_scope_contexts[t_index].size()-1) {
							switch (it->first->type) {
							case NODE_TYPE_ACTION:
								{
									ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
									test_inputs[d_index].push_back(action_node_history->obs_snapshot[test_input_obs_indexes[t_index]]);
								}
								break;
							case NODE_TYPE_BRANCH:
								{
									BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
									if (branch_node_history->is_branch) {
										test_inputs[d_index].push_back(1.0);
									} else {
										test_inputs[d_index].push_back(-1.0);
									}
								}
								break;
							case NODE_TYPE_INFO_SCOPE:
								{
									InfoScopeNodeHistory* info_scope_node_history = (InfoScopeNodeHistory*)it->second;
									if (info_scope_node_history->is_positive) {
										test_inputs[d_index].push_back(1.0);
									} else {
										test_inputs[d_index].push_back(-1.0);
									}
								}
								break;
							case NODE_TYPE_INFO_BRANCH:
								{
									InfoBranchNodeHistory* info_branch_node_history = (InfoBranchNodeHistory*)it->second;
									if (info_branch_node_history->is_branch) {
										test_inputs[d_index].push_back(1.0);
									} else {
										test_inputs[d_index].push_back(-1.0);
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
		}

		train_w_drop_network(test_inputs,
							 target_val_histories,
							 test_network);

		double test_average_misguess;
		double test_misguess_standard_deviation;
		measure_network(test_inputs,
						target_val_histories,
						test_network,
						test_average_misguess,
						test_misguess_standard_deviation);

		bool is_select = false;
		if (parent_scope->eval_network == NULL) {
			is_select = true;
		} else {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%3 == 0) {
			#else
			double improvement = average_misguess - test_average_misguess;
			double standard_deviation = min(misguess_standard_deviation, test_misguess_standard_deviation);
			double t_score = improvement / (standard_deviation / sqrt(num_instances * TEST_SAMPLES_PERCENTAGE));

			if (t_score > 1.645) {
			#endif /* MDEBUG */
				is_select = true;
			}
		}

		if (is_select) {
			int original_input_size = (int)parent_scope->eval_input_node_contexts.size();
			int test_input_size = (int)test_input_node_contexts.size();

			average_misguess = test_average_misguess;
			misguess_standard_deviation = test_misguess_standard_deviation;

			parent_scope->eval_input_scope_contexts = test_input_scope_contexts;
			parent_scope->eval_input_node_contexts = test_input_node_contexts;
			parent_scope->eval_input_obs_indexes = test_input_obs_indexes;

			if (parent_scope->eval_network != NULL) {
				delete parent_scope->eval_network;
			}
			parent_scope->eval_network = test_network;

			inputs = test_inputs;

			/**
			 * - only clean new as old might have been dependent on previous datapoints
			 * 
			 * - removes both irrelevant as well as duplicates
			 */
			for (int i_index = test_input_size-1; i_index >= original_input_size; i_index--) {
				vector<vector<Scope*>> remove_test_input_scope_contexts = parent_scope->eval_input_scope_contexts;
				vector<vector<AbstractNode*>> remove_test_input_node_contexts = parent_scope->eval_input_node_contexts;
				vector<int> remove_test_input_obs_indexes = parent_scope->eval_input_obs_indexes;

				remove_test_input_scope_contexts.erase(remove_test_input_scope_contexts.begin() + i_index);
				remove_test_input_node_contexts.erase(remove_test_input_node_contexts.begin() + i_index);
				remove_test_input_obs_indexes.erase(remove_test_input_obs_indexes.begin() + i_index);

				Network* remove_test_network = new Network(parent_scope->eval_network);

				remove_test_network->input->acti_vals.erase(remove_test_network->input->acti_vals.begin() + i_index);
				remove_test_network->input->errors.erase(remove_test_network->input->errors.begin() + i_index);

				for (int l_index = 0; l_index < (int)remove_test_network->hiddens.size(); l_index++) {
					remove_test_network->hiddens[l_index]->remove_input(i_index);
				}
				remove_test_network->output->remove_input(i_index);

				vector<vector<double>> remove_test_inputs = inputs;

				for (int d_index = 0; d_index < num_instances; d_index++) {
					remove_test_inputs[d_index].erase(remove_test_inputs[d_index].begin() + i_index);
				}

				optimize_w_drop_network(remove_test_inputs,
										target_val_histories,
										remove_test_network);

				double remove_test_average_misguess;
				double remove_test_misguess_standard_deviation;
				measure_network(remove_test_inputs,
								target_val_histories,
								remove_test_network,
								remove_test_average_misguess,
								remove_test_misguess_standard_deviation);

				double remove_improvement = average_misguess - remove_test_average_misguess;
				double remove_standard_deviation = min(misguess_standard_deviation, remove_test_misguess_standard_deviation);
				double remove_t_score = remove_improvement / (remove_standard_deviation / sqrt(num_instances * TEST_SAMPLES_PERCENTAGE));

				if (remove_t_score > -0.674) {
					parent_scope->eval_input_scope_contexts = remove_test_input_scope_contexts;
					parent_scope->eval_input_node_contexts = remove_test_input_node_contexts;
					parent_scope->eval_input_obs_indexes = remove_test_input_obs_indexes;

					delete parent_scope->eval_network;
					parent_scope->eval_network = remove_test_network;

					inputs = remove_test_inputs;
				} else {
					delete remove_test_network;
				}
			}

			#if defined(MDEBUG) && MDEBUG
			#else
			train_index = 0;
			#endif /* MDEBUG */
		} else {
			delete test_network;

			train_index++;
		}
	}

	parent_scope->eval_input_scope_context_ids.clear();
	parent_scope->eval_input_node_context_ids.clear();
	for (int i_index = 0; i_index < (int)parent_scope->eval_input_scope_contexts.size(); i_index++) {
		vector<int> scope_context_ids(parent_scope->eval_input_scope_contexts[i_index].size());
		for (int v_index = 0; v_index < (int)parent_scope->eval_input_scope_contexts[i_index].size(); v_index++) {
			scope_context_ids[v_index] = parent_scope->eval_input_scope_contexts[i_index][v_index]->id;
		}
		parent_scope->eval_input_scope_context_ids.push_back(scope_context_ids);

		vector<int> node_context_ids(parent_scope->eval_input_node_contexts[i_index].size());
		for (int v_index = 0; v_index < (int)parent_scope->eval_input_node_contexts[i_index].size(); v_index++) {
			node_context_ids[v_index] = parent_scope->eval_input_node_contexts[i_index][v_index]->id;
		}
		parent_scope->eval_input_node_context_ids.push_back(node_context_ids);
	}

	for (int h_index = 0; h_index < (int)scope_histories.size(); h_index++) {
		delete scope_histories[h_index];
	}
}
