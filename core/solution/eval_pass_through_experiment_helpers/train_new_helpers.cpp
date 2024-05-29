#include "eval_pass_through_experiment.h"

#include <cmath>
#include <iostream>

#include "action_node.h"
#include "constants.h"
#include "eval.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_scope_node.h"
#include "network.h"
#include "nn_helpers.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

void EvalPassThroughExperiment::train_new() {
	vector<vector<double>> inputs(NUM_DATAPOINTS);
	double average_misguess;
	double misguess_standard_deviation;
	if (this->network != NULL) {
		for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
			vector<double> d_inputs(this->input_node_contexts.size(), 0.0);
			for (int i_index = 0; i_index < (int)this->input_node_contexts.size(); i_index++) {
				map<AbstractNode*, AbstractNodeHistory*>::iterator it = this->eval_histories[d_index]->scope_history->node_histories.find(
					this->input_node_contexts[i_index]);
				if (it != this->eval_histories[d_index]->scope_history->node_histories.end()) {
					switch (this->input_node_contexts[i_index]->type) {
					case NODE_TYPE_ACTION:
						{
							ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
							d_inputs[i_index] = action_node_history->obs_snapshot[this->input_obs_indexes[i_index]];
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
				}
			}
			inputs[d_index] = d_inputs;
		}

		optimize_network(inputs,
						 this->target_val_histories,
						 this->network);

		double average_misguess;
		double misguess_standard_deviation;
		measure_network(inputs,
						this->target_val_histories,
						this->network,
						average_misguess,
						misguess_standard_deviation);
	}

	cout << "starting this->input_node_contexts.size(): " << this->input_node_contexts.size() << endl;

	int train_index = 0;
	while (train_index < 3) {
		vector<AbstractNode*> test_input_node_contexts = this->input_node_contexts;
		vector<int> test_input_obs_indexes = this->input_obs_indexes;

		vector<AbstractNode*> possible_node_contexts;
		vector<int> possible_obs_indexes;

		uniform_int_distribution<int> history_distribution(0, NUM_DATAPOINTS - 1);
		gather_eval_possible_helper(possible_node_contexts,
									possible_obs_indexes,
									this->eval_histories[history_distribution(generator)]);

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
		if (this->network == NULL) {
			test_network = new Network();
		} else {
			test_network = new Network(this->network);
		}
		test_network->increment((int)test_input_node_contexts.size());

		vector<vector<double>> test_inputs = inputs;
		for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
			test_inputs[d_index].reserve((int)test_input_node_contexts.size());
			for (int t_index = (int)this->input_node_contexts.size(); t_index < (int)test_input_node_contexts.size(); t_index++) {
				map<AbstractNode*, AbstractNodeHistory*>::iterator it = this->eval_histories[d_index]->scope_history->node_histories.find(
					test_input_node_contexts[t_index]);
				if (it == this->eval_histories[d_index]->scope_history->node_histories.end()) {
					test_inputs[d_index].push_back(0.0);
				} else {
					switch (test_input_node_contexts[t_index]->type) {
					case NODE_TYPE_ACTION:
						{
							ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
							test_inputs[d_index].push_back(action_node_history->obs_snapshot[test_input_obs_indexes[t_index]]);
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
				}
			}
		}

		train_network(test_inputs,
					  this->target_val_histories,
					  test_network);

		double test_average_misguess;
		double test_misguess_standard_deviation;
		measure_network(test_inputs,
						this->target_val_histories,
						test_network,
						test_average_misguess,
						test_misguess_standard_deviation);

		cout << "test_average_misguess: " << test_average_misguess << endl;
		cout << "test_misguess_standard_deviation: " << test_misguess_standard_deviation << endl;

		bool is_select = false;
		if (this->network == NULL) {
			is_select = true;
		} else {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%3 == 0) {
			#else
			double improvement = average_misguess - test_average_misguess;
			double standard_deviation = min(misguess_standard_deviation, test_misguess_standard_deviation);
			double t_score = improvement / (standard_deviation / sqrt(NUM_DATAPOINTS * TEST_SAMPLES_PERCENTAGE));

			if (t_score > 1.645) {
			#endif /* MDEBUG */
				is_select = true;
			}
		}

		if (is_select) {
			cout << "select" << endl;
			int original_input_size = (int)this->input_node_contexts.size();
			int test_input_size = (int)test_input_node_contexts.size();

			average_misguess = test_average_misguess;
			misguess_standard_deviation = test_misguess_standard_deviation;

			this->input_node_contexts = test_input_node_contexts;
			this->input_obs_indexes = test_input_obs_indexes;

			if (this->network != NULL) {
				delete this->network;
			}
			this->network = test_network;

			inputs = test_inputs;

			/**
			 * - only clean new as old might have been dependent on previous datapoints
			 */
			for (int i_index = test_input_size-1; i_index >= original_input_size; i_index--) {
				vector<AbstractNode*> remove_test_input_node_contexts = this->input_node_contexts;
				vector<int> remove_test_input_obs_indexes = this->input_obs_indexes;

				remove_test_input_node_contexts.erase(remove_test_input_node_contexts.begin() + i_index);
				remove_test_input_obs_indexes.erase(remove_test_input_obs_indexes.begin() + i_index);

				Network* remove_test_network = new Network(this->network);

				remove_test_network->input->acti_vals.erase(remove_test_network->input->acti_vals.begin() + i_index);
				remove_test_network->input->errors.erase(remove_test_network->input->errors.begin() + i_index);

				for (int l_index = 0; l_index < (int)remove_test_network->hiddens.size(); l_index++) {
					remove_test_network->hiddens[l_index]->remove_input(i_index);
				}
				remove_test_network->output->remove_input(i_index);

				vector<vector<double>> remove_test_inputs = inputs;

				for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
					remove_test_inputs[d_index].erase(remove_test_inputs[d_index].begin() + i_index);
				}

				optimize_network(remove_test_inputs,
								 this->target_val_histories,
								 remove_test_network);

				double remove_test_average_misguess;
				double remove_test_misguess_standard_deviation;
				measure_network(remove_test_inputs,
								this->target_val_histories,
								remove_test_network,
								remove_test_average_misguess,
								remove_test_misguess_standard_deviation);

				double remove_improvement = average_misguess - remove_test_average_misguess;
				double remove_standard_deviation = min(misguess_standard_deviation, remove_test_misguess_standard_deviation);
				double remove_t_score = remove_improvement / (remove_standard_deviation / sqrt(NUM_DATAPOINTS * TEST_SAMPLES_PERCENTAGE));

				if (remove_t_score > -0.674) {
					this->input_node_contexts = remove_test_input_node_contexts;
					this->input_obs_indexes = remove_test_input_obs_indexes;

					delete this->network;
					this->network = remove_test_network;

					inputs = remove_test_inputs;
				} else {
					this->network = remove_test_network;
				}
			}

			cout << "updated this->input_node_contexts.size(): " << this->input_node_contexts.size() << endl;

			#if defined(MDEBUG) && MDEBUG
			#else
			train_index = 0;
			#endif /* MDEBUG */
		} else {
			delete test_network;

			train_index++;
		}
	}
}
