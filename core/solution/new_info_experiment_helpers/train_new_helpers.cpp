#include "new_info_experiment.h"

#include <algorithm>
#include <cmath>
#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "eval_helpers.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_scope.h"
#include "network.h"
#include "nn_helpers.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

bool NewInfoExperiment::train_new_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		NewInfoExperimentHistory* history) {
	if (run_helper.branch_node_ancestors.find(this->branch_node) != run_helper.branch_node_ancestors.end()) {
		return false;
	}

	run_helper.branch_node_ancestors.insert(this->branch_node);

	run_helper.num_decisions++;

	InfoBranchNodeHistory* branch_node_history = new InfoBranchNodeHistory();
	branch_node_history->index = context.back().scope_history->node_histories.size();
	context.back().scope_history->node_histories[this->branch_node] = branch_node_history;

	AbstractScopeHistory* scope_history;
	this->new_info_scope->explore_activate(problem,
										   context,
										   run_helper,
										   scope_history);

	this->num_instances_until_target--;
	if (this->num_instances_until_target == 0) {
		this->scope_histories.push_back(scope_history);

		switch (this->score_type) {
		case SCORE_TYPE_TRUTH:
			history->predicted_scores.push_back(vector<double>());
			break;
		case SCORE_TYPE_ALL:
			history->predicted_scores.push_back(vector<double>(context.size()-1, 0.0));
			for (int l_index = 0; l_index < (int)context.size()-1; l_index++) {
				ScopeHistory* scope_history = (ScopeHistory*)context[l_index].scope_history;
				Scope* scope = (Scope*)scope_history->scope;

				if (scope->eval_network != NULL) {
					scope_history->callback_experiment_history = history;
					scope_history->callback_experiment_indexes.push_back(
						(int)history->predicted_scores.size()-1);
					scope_history->callback_experiment_layers.push_back(l_index);
				}
			}
			break;
		}

		vector<double> input_vals(this->existing_input_node_contexts.size(), 0.0);
		for (int i_index = 0; i_index < (int)this->existing_input_node_contexts.size(); i_index++) {
			map<AbstractNode*, AbstractNodeHistory*>::iterator it = scope_history->node_histories.find(
				this->existing_input_node_contexts[i_index]);
			if (it != scope_history->node_histories.end()) {
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
				input_vals[i_index] = action_node_history->obs_snapshot[this->existing_input_obs_indexes[i_index]];
			}
		}
		this->existing_network->activate(input_vals);
		double predicted_score = this->existing_network->output->acti_vals[0];

		history->existing_predicted_scores.push_back(predicted_score);

		if (this->best_step_types.size() == 0) {
			curr_node = this->best_exit_next_node;
		} else {
			if (this->best_step_types[0] == STEP_TYPE_ACTION) {
				curr_node = this->best_actions[0];
			} else {
				curr_node = this->best_scopes[0];
			}
		}

		uniform_int_distribution<int> until_distribution(0, (int)this->average_instances_per_run-1.0);
		this->num_instances_until_target = 1 + until_distribution(generator);

		return true;
	} else {
		delete scope_history;

		return false;
	}
}

void NewInfoExperiment::train_new_back_activate(
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	NewInfoExperimentHistory* history = (NewInfoExperimentHistory*)run_helper.experiment_histories.back();

	ScopeHistory* scope_history = (ScopeHistory*)context.back().scope_history;

	double predicted_score;
	if (run_helper.exceeded_limit) {
		predicted_score = -1.0;
	} else {
		predicted_score = calc_score(scope_history) / (int)scope_history->callback_experiment_indexes.size();
	}
	for (int i_index = 0; i_index < (int)scope_history->callback_experiment_indexes.size(); i_index++) {
		history->predicted_scores[scope_history->callback_experiment_indexes[i_index]]
			[scope_history->callback_experiment_layers[i_index]] = predicted_score;
	}
}

void NewInfoExperiment::train_new_backprop(
		double target_val,
		RunHelper& run_helper) {
	NewInfoExperimentHistory* history = (NewInfoExperimentHistory*)run_helper.experiment_histories.back();

	for (int i_index = 0; i_index < (int)history->predicted_scores.size(); i_index++) {
		double final_score;
		switch (this->score_type) {
		case SCORE_TYPE_TRUTH:
			if (run_helper.exceeded_limit) {
				final_score = -10.0;
			} else {
				final_score = (target_val - solution->average_score) / (int)history->predicted_scores.size();
			}
			break;
		case SCORE_TYPE_ALL:
			{
				double sum_score;
				if (run_helper.exceeded_limit) {
					sum_score = -10.0;
				} else {
					sum_score = (target_val - solution->average_score) / (int)history->predicted_scores.size();
				}
				for (int l_index = 0; l_index < (int)history->predicted_scores[i_index].size(); l_index++) {
					sum_score += history->predicted_scores[i_index][l_index];
				}
				final_score = sum_score / ((int)history->predicted_scores[i_index].size() + 1);
			}
			break;
		}

		double surprise = final_score - history->existing_predicted_scores[i_index];

		this->target_val_histories.push_back(surprise);
	}

	this->state_iter++;
	if ((int)this->target_val_histories.size() >= NUM_DATAPOINTS
			&& this->state_iter >= MIN_NUM_TRUTH_DATAPOINTS) {
		default_random_engine generator_copy = generator;
		shuffle(this->scope_histories.begin(), this->scope_histories.end(), generator);
		shuffle(this->target_val_histories.begin(), this->target_val_histories.end(), generator_copy);

		int num_instances = (int)this->target_val_histories.size();

		double sum_scores = 0.0;
		for (int d_index = 0; d_index < num_instances; d_index++) {
			sum_scores += this->target_val_histories[d_index];
		}
		this->new_average_score = sum_scores / num_instances;

		vector<vector<double>> inputs(num_instances);
		#if defined(MDEBUG) && MDEBUG
		#else
		double average_misguess;
		double misguess_standard_deviation;
		#endif /* MDEBUG */

		int train_index = 0;
		while (train_index < 3) {
			vector<AbstractNode*> possible_node_contexts;
			vector<int> possible_obs_indexes;

			uniform_int_distribution<int> history_distribution(0, num_instances-1);
			AbstractScopeHistory* scope_history = this->scope_histories[history_distribution(generator)];
			for (map<AbstractNode*, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
					it != scope_history->node_histories.end(); it++) {
				for (int o_index = 0; o_index < problem_type->num_obs(); o_index++) {
					possible_node_contexts.push_back(it->first);
					possible_obs_indexes.push_back(o_index);
				}
			}

			if (possible_node_contexts.size() > 0) {
				vector<AbstractNode*> test_input_node_contexts = this->new_input_node_contexts;
				vector<int> test_input_obs_indexes = this->new_input_obs_indexes;

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
				if (this->new_network == NULL) {
					test_network = new Network();
				} else {
					test_network = new Network(this->new_network);
				}
				test_network->increment((int)test_input_node_contexts.size());

				vector<vector<double>> test_inputs = inputs;
				for (int d_index = 0; d_index < num_instances; d_index++) {
					test_inputs[d_index].reserve((int)test_input_node_contexts.size());
					for (int t_index = (int)this->new_input_node_contexts.size(); t_index < (int)test_input_node_contexts.size(); t_index++) {
						map<AbstractNode*, AbstractNodeHistory*>::iterator it = this->scope_histories[d_index]->node_histories.find(
							test_input_node_contexts[t_index]);
						if (it == this->scope_histories[d_index]->node_histories.end()) {
							test_inputs[d_index].push_back(0.0);
						} else {
							ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
							test_inputs[d_index].push_back(action_node_history->obs_snapshot[test_input_obs_indexes[t_index]]);
						}
					}
				}

				train_w_drop_network(test_inputs,
									 this->target_val_histories,
									 test_network);

				double test_average_misguess;
				double test_misguess_standard_deviation;
				measure_network(test_inputs,
								this->target_val_histories,
								test_network,
								test_average_misguess,
								test_misguess_standard_deviation);

				bool is_select = false;
				if (this->new_network == NULL) {
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

					int original_input_size = (int)this->new_input_node_contexts.size();
					int test_input_size = (int)test_input_node_contexts.size();

					#if defined(MDEBUG) && MDEBUG
					#else
					average_misguess = test_average_misguess;
					misguess_standard_deviation = test_misguess_standard_deviation;
					#endif /* MDEBUG */

					this->new_input_node_contexts = test_input_node_contexts;
					this->new_input_obs_indexes = test_input_obs_indexes;

					if (this->new_network != NULL) {
						delete this->new_network;
					}
					this->new_network = test_network;

					inputs = test_inputs;

					for (int i_index = test_input_size-1; i_index >= original_input_size; i_index--) {
						vector<AbstractNode*> remove_test_input_node_contexts = this->new_input_node_contexts;
						vector<int> remove_test_input_obs_indexes = this->new_input_obs_indexes;

						remove_test_input_node_contexts.erase(remove_test_input_node_contexts.begin() + i_index);
						remove_test_input_obs_indexes.erase(remove_test_input_obs_indexes.begin() + i_index);

						Network* remove_test_network = new Network(this->new_network);
						remove_test_network->remove_input(i_index);

						vector<vector<double>> remove_test_inputs = inputs;
						for (int d_index = 0; d_index < num_instances; d_index++) {
							remove_test_inputs[d_index].erase(remove_test_inputs[d_index].begin() + i_index);
						}

						optimize_w_drop_network(remove_test_inputs,
												this->target_val_histories,
												remove_test_network);

						double remove_test_average_misguess;
						double remove_test_misguess_standard_deviation;
						measure_network(remove_test_inputs,
										this->target_val_histories,
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
							this->new_input_node_contexts = remove_test_input_node_contexts;
							this->new_input_obs_indexes = remove_test_input_obs_indexes;

							delete this->new_network;
							this->new_network = remove_test_network;

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

		for (int i_index = 0; i_index < (int)this->scope_histories.size(); i_index++) {
			delete this->scope_histories[i_index];
		}
		this->scope_histories.clear();
		this->target_val_histories.clear();

		this->combined_score = 0.0;
		this->original_count = 0;
		this->branch_count = 0;

		this->state = NEW_INFO_EXPERIMENT_STATE_MEASURE;
		this->state_iter = 0;
		this->sub_state_iter = 0;
	}
}
