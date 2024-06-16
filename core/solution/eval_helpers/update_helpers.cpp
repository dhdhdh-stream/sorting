#include "eval_helpers.h"

#include <algorithm>
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
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

void update_eval_helper(Scope* parent_scope,
						vector<ScopeHistory*>& scope_histories,
						vector<double>& target_val_histories) {
	vector<double> target_vals(target_val_histories.size());
	for (int d_index = 0; d_index < (int)target_val_histories.size(); d_index++) {
		target_vals[d_index] = target_val_histories[d_index] - solution->average_score;
	}

	default_random_engine generator_copy = generator;
	shuffle(scope_histories.begin(), scope_histories.end(), generator);
	shuffle(target_vals.begin(), target_vals.end(), generator_copy);

	int num_instances = (int)scope_histories.size();

	vector<vector<double>> inputs(num_instances);
	double average_misguess;
	double misguess_standard_deviation;
	if (parent_scope->eval_network != NULL) {
		for (int d_index = 0; d_index < num_instances; d_index++) {
			vector<double> d_inputs(parent_scope->eval_input_node_contexts.size(), 0.0);
			for (int i_index = 0; i_index < (int)parent_scope->eval_input_node_contexts.size(); i_index++) {
				map<AbstractNode*, AbstractNodeHistory*>::iterator it = scope_histories[d_index]->node_histories.find(
					parent_scope->eval_input_node_contexts[i_index]);
				if (it != scope_histories[d_index]->node_histories.end()) {
					switch (it->first->type) {
					case NODE_TYPE_ACTION:
						{
							ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
							d_inputs[i_index] = action_node_history->obs_snapshot[parent_scope->eval_input_obs_indexes[i_index]];
						}
						break;
					case NODE_TYPE_SCOPE:
						{
							ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
							d_inputs[i_index] = scope_node_history->obs_snapshot[parent_scope->eval_input_obs_indexes[i_index]];
						}
						break;
					case NODE_TYPE_BRANCH:
						{
							BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
							d_inputs[i_index] = branch_node_history->score;
						}
						break;
					case NODE_TYPE_INFO_BRANCH:
						{
							InfoBranchNodeHistory* info_branch_node_history = (InfoBranchNodeHistory*)it->second;
							d_inputs[i_index] = info_branch_node_history->score;
						}
						break;
					}
				}
			}
			inputs[d_index] = d_inputs;
		}

		optimize_w_drop_network(inputs,
								target_vals,
								parent_scope->eval_network);

		measure_network(inputs,
						target_vals,
						parent_scope->eval_network,
						average_misguess,
						misguess_standard_deviation);
	}

	int train_index = 0;
	while (train_index < 3) {
		vector<AbstractNode*> possible_node_contexts;
		vector<int> possible_obs_indexes;

		uniform_int_distribution<int> history_distribution(0, num_instances-1);
		gather_possible_helper(possible_node_contexts,
							   possible_obs_indexes,
							   scope_histories[history_distribution(generator)]);

		if (possible_node_contexts.size() > 0) {
			vector<AbstractNode*> test_input_node_contexts = parent_scope->eval_input_node_contexts;
			vector<int> test_input_obs_indexes = parent_scope->eval_input_obs_indexes;

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
					if (possible_node_contexts[remaining_indexes[rand_index]] == test_input_node_contexts[i_index]
							&& possible_obs_indexes[remaining_indexes[rand_index]] == test_input_obs_indexes[i_index]) {
						contains = true;
						break;
					}
				}
				if (!contains) {
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
					map<AbstractNode*, AbstractNodeHistory*>::iterator it = scope_histories[d_index]->node_histories.find(
						test_input_node_contexts[t_index]);
					if (it == scope_histories[d_index]->node_histories.end()) {
						test_inputs[d_index].push_back(0.0);
					} else {
						switch (it->first->type) {
						case NODE_TYPE_ACTION:
							{
								ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
								test_inputs[d_index].push_back(action_node_history->obs_snapshot[test_input_obs_indexes[t_index]]);
							}
							break;
						case NODE_TYPE_SCOPE:
							{
								ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
								test_inputs[d_index].push_back(scope_node_history->obs_snapshot[test_input_obs_indexes[t_index]]);
							}
							break;
						case NODE_TYPE_BRANCH:
							{
								BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
								test_inputs[d_index].push_back(branch_node_history->score);
							}
							break;
						case NODE_TYPE_INFO_BRANCH:
							{
								InfoBranchNodeHistory* info_branch_node_history = (InfoBranchNodeHistory*)it->second;
								test_inputs[d_index].push_back(info_branch_node_history->score);
							}
							break;
						}
					}
				}
			}

			train_w_drop_network(test_inputs,
								 target_vals,
								 test_network);

			double test_average_misguess;
			double test_misguess_standard_deviation;
			measure_network(test_inputs,
							target_vals,
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
				cout << "s" << endl;

				int original_input_size = (int)parent_scope->eval_input_node_contexts.size();
				int test_input_size = (int)test_input_node_contexts.size();

				average_misguess = test_average_misguess;
				misguess_standard_deviation = test_misguess_standard_deviation;

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
					vector<AbstractNode*> remove_test_input_node_contexts = parent_scope->eval_input_node_contexts;
					vector<int> remove_test_input_obs_indexes = parent_scope->eval_input_obs_indexes;

					remove_test_input_node_contexts.erase(remove_test_input_node_contexts.begin() + i_index);
					remove_test_input_obs_indexes.erase(remove_test_input_obs_indexes.begin() + i_index);

					Network* remove_test_network = new Network(parent_scope->eval_network);
					remove_test_network->remove_input(i_index);

					vector<vector<double>> remove_test_inputs = inputs;
					for (int d_index = 0; d_index < num_instances; d_index++) {
						remove_test_inputs[d_index].erase(remove_test_inputs[d_index].begin() + i_index);
					}

					optimize_w_drop_network(remove_test_inputs,
											target_vals,
											remove_test_network);

					double remove_test_average_misguess;
					double remove_test_misguess_standard_deviation;
					measure_network(remove_test_inputs,
									target_vals,
									remove_test_network,
									remove_test_average_misguess,
									remove_test_misguess_standard_deviation);

					#if defined(MDEBUG) && MDEBUG
					if (rand()%2 == 0) {
					#else
					double remove_improvement = average_misguess - remove_test_average_misguess;
					double remove_standard_deviation = min(misguess_standard_deviation, remove_test_misguess_standard_deviation);
					double remove_t_score = remove_improvement / (remove_standard_deviation / sqrt(num_instances * TEST_SAMPLES_PERCENTAGE));

					if (remove_t_score > -0.674) {
					#endif /* MDEBUG */
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
		} else {
			train_index++;
		}

		cout << "i" << endl;
	}

	for (int h_index = 0; h_index < (int)scope_histories.size(); h_index++) {
		delete scope_histories[h_index];
	}
}

void update_eval() {
	if (solution->last_updated_scope_id != -1
			|| solution->last_new_scope_id != -1) {
		Scope* last_update_scope;
		if (solution->last_updated_scope_id != -1) {
			last_update_scope = solution->scopes[solution->last_updated_scope_id];
		} else {
			last_update_scope = NULL;
		}

		Scope* last_new_scope;
		if (solution->last_new_scope_id != -1) {
			last_new_scope = solution->scopes[solution->last_new_scope_id];
		} else {
			last_new_scope = NULL;
		}

		vector<ScopeHistory*> scope_histories;
		vector<double> target_val_histories;
		vector<ScopeHistory*> new_scope_histories;
		vector<double> new_target_val_histories;
		for (int iter_index = 0; iter_index < MEASURE_ITERS; iter_index++) {
			Problem* problem = problem_type->get_problem();

			RunHelper run_helper;
			#if defined(MDEBUG) && MDEBUG
			run_helper.curr_run_seed = run_index;
			run_index++;
			#endif /* MDEBUG */

			Metrics metrics;
			metrics.experiment_scope = last_update_scope;
			metrics.new_scope = last_new_scope;

			vector<ContextLayer> context;
			solution->scopes[0]->measure_activate(
				metrics,
				problem,
				context,
				run_helper);

			double target_val;
			if (!run_helper.exceeded_limit) {
				target_val = problem->score_result(run_helper.num_decisions);
			} else {
				target_val = -1.0;
			}

			for (int h_index = 0; h_index < (int)metrics.scope_histories.size(); h_index++) {
				scope_histories.push_back(metrics.scope_histories[h_index]);
				target_val_histories.push_back(target_val);
			}
			for (int h_index = 0; h_index < (int)metrics.new_scope_histories.size(); h_index++) {
				new_scope_histories.push_back(metrics.new_scope_histories[h_index]);
				new_target_val_histories.push_back(target_val);
			}

			delete problem;
		}

		update_eval_helper(last_update_scope,
						   scope_histories,
						   target_val_histories);
		if (last_new_scope != NULL) {
			update_eval_helper(last_new_scope,
							   new_scope_histories,
							   new_target_val_histories);
		}
	}
}
