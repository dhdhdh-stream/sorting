#include "new_info_experiment.h"

#include <algorithm>
#include <iostream>

#include "action_node.h"
#include "constants.h"
#include "eval_helpers.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_scope_node.h"
#include "network.h"
#include "nn_helpers.h"
#include "scope.h"
#include "solution_helpers.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int INITIAL_NUM_SAMPLES_PER_ITER = 2;
const int INITIAL_NUM_TRUTH_PER_ITER = 2;
const int VERIFY_1ST_NUM_SAMPLES_PER_ITER = 5;
const int VERIFY_1ST_NUM_TRUTH_PER_ITER = 2;
const int VERIFY_2ND_NUM_SAMPLES_PER_ITER = 10;
const int VERIFY_2ND_NUM_TRUTH_PER_ITER = 2;
const int EXPLORE_ITERS = 2;
#else
const int INITIAL_NUM_SAMPLES_PER_ITER = 100;
const int INITIAL_NUM_TRUTH_PER_ITER = 5;
const int VERIFY_1ST_NUM_SAMPLES_PER_ITER = 500;
const int VERIFY_1ST_NUM_TRUTH_PER_ITER = 25;
const int VERIFY_2ND_NUM_SAMPLES_PER_ITER = 2000;
const int VERIFY_2ND_NUM_TRUTH_PER_ITER = 100;
const int EXPLORE_ITERS = 100;
#endif /* MDEBUG */

void NewInfoExperiment::explore_info_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		NewInfoExperimentHistory* history) {
	run_helper.num_decisions++;

	ScopeHistory* scope_history;
	this->new_info_subscope->info_activate(problem,
										   run_helper,
										   scope_history);

	this->scope_histories.push_back(scope_history);

	history->predicted_scores.push_back(vector<double>(context.size(), 0.0));
	for (int l_index = 0; l_index < (int)context.size(); l_index++) {
		if (context[l_index].scope->eval_network != NULL) {
			context[l_index].scope_history->callback_experiment_history = history;
			context[l_index].scope_history->callback_experiment_indexes.push_back(
				(int)history->predicted_scores.size()-1);
			context[l_index].scope_history->callback_experiment_layers.push_back(l_index);
		}
	}
}

void NewInfoExperiment::explore_info_back_activate(
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	NewInfoExperimentHistory* history = (NewInfoExperimentHistory*)run_helper.experiment_histories.back();

	double predicted_score;
	if (run_helper.exceeded_limit) {
		predicted_score = -1.0;
	} else {
		predicted_score = calc_score(context.back().scope_history);
	}
	for (int i_index = 0; i_index < (int)context.back().scope_history->callback_experiment_indexes.size(); i_index++) {
		history->predicted_scores[context.back().scope_history->callback_experiment_indexes[i_index]]
			[context.back().scope_history->callback_experiment_layers[i_index]] = predicted_score;
	}
}

void NewInfoExperiment::explore_info_backprop(
		double target_val,
		RunHelper& run_helper) {
	bool is_fail = false;

	if (run_helper.exceeded_limit) {
		is_fail = true;
	} else {
		NewInfoExperimentHistory* history = (NewInfoExperimentHistory*)run_helper.experiment_histories.back();

		this->sub_state_iter++;
		if (this->sub_state_iter == INITIAL_NUM_TRUTH_PER_ITER
				&& (int)this->target_val_histories.size() >= INITIAL_NUM_SAMPLES_PER_ITER) {
			#if defined(MDEBUG) && MDEBUG
			if (false) {
			#else
			if (this->info_score < 0.0) {
			#endif /* MDEBUG */
				is_fail = true;
			}
		} else if (this->sub_state_iter == VERIFY_1ST_NUM_TRUTH_PER_ITER
				&& (int)this->target_val_histories.size() >= VERIFY_1ST_NUM_SAMPLES_PER_ITER) {
			#if defined(MDEBUG) && MDEBUG
			if (false) {
			#else
			if (this->info_score < 0.0) {
			#endif /* MDEBUG */
				is_fail = true;
			}
		} else if (this->sub_state_iter == VERIFY_2ND_NUM_TRUTH_PER_ITER
				&& (int)this->target_val_histories.size() >= VERIFY_2ND_NUM_SAMPLES_PER_ITER) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			if (this->info_score < 0.0) {
			#endif /* MDEBUG */
				is_fail = true;
			}
		}

		for (int i_index = 0; i_index < (int)history->predicted_scores.size(); i_index++) {
			double sum_score = 0.0;
			for (int l_index = 0; l_index < (int)history->predicted_scores[i_index].size(); l_index++) {
				sum_score += history->predicted_scores[i_index][l_index];
			}
			double final_score = (sum_score / (int)history->predicted_scores[i_index].size() + target_val) / 2.0;

			this->info_score += final_score - this->existing_average_score;

			this->target_val_histories.push_back(final_score);

			if ((int)this->target_val_histories.size() == INITIAL_NUM_SAMPLES_PER_ITER
					&& this->sub_state_iter >= INITIAL_NUM_TRUTH_PER_ITER) {
				#if defined(MDEBUG) && MDEBUG
				if (false) {
				#else
				if (this->info_score < 0.0) {
				#endif /* MDEBUG */
					is_fail = true;
				}
			} else if ((int)this->target_val_histories.size() == VERIFY_1ST_NUM_SAMPLES_PER_ITER
					&& this->sub_state_iter >= VERIFY_1ST_NUM_TRUTH_PER_ITER) {
				#if defined(MDEBUG) && MDEBUG
				if (false) {
				#else
				if (this->info_score < 0.0) {
				#endif /* MDEBUG */
					is_fail = true;
				}
			} else if ((int)this->target_val_histories.size() == VERIFY_2ND_NUM_SAMPLES_PER_ITER
					&& this->sub_state_iter >= VERIFY_2ND_NUM_TRUTH_PER_ITER) {
				#if defined(MDEBUG) && MDEBUG
				if (rand()%2 == 0) {
				#else
				if (this->info_score < 0.0) {
				#endif /* MDEBUG */
					is_fail = true;
				}
			}
		}
	}

	if (is_fail) {
		delete this->new_info_subscope;
		this->new_info_subscope = NULL;

		for (int h_index = 0; h_index < (int)this->scope_histories.size(); h_index++) {
			delete this->scope_histories[h_index];
		}
		this->scope_histories.clear();
		this->target_val_histories.clear();

		this->state_iter++;
		if (this->state_iter >= EXPLORE_ITERS) {
			this->result = EXPERIMENT_RESULT_FAIL;
		} else {
			this->info_score = 0.0;
			this->new_info_subscope = create_new_info_subscope();

			this->scope_histories.reserve(NUM_DATAPOINTS);
			this->target_val_histories.reserve(NUM_DATAPOINTS);

			this->sub_state_iter = 0;
		}
	} else if ((int)this->target_val_histories.size() >= VERIFY_2ND_NUM_SAMPLES_PER_ITER
			&& this->sub_state_iter >= VERIFY_2ND_NUM_TRUTH_PER_ITER) {
		default_random_engine generator_copy = generator;
		shuffle(this->scope_histories.begin(), this->scope_histories.end(), generator);
		shuffle(this->target_val_histories.begin(), this->target_val_histories.end(), generator_copy);

		int num_instances = (int)this->target_val_histories.size();

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
			gather_possible_helper(possible_node_contexts,
								   possible_obs_indexes,
								   this->scope_histories[history_distribution(generator)]);

			if (possible_node_contexts.size() > 0) {
				vector<AbstractNode*> test_input_node_contexts = this->existing_input_node_contexts;
				vector<int> test_input_obs_indexes = this->existing_input_obs_indexes;

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
				if (this->existing_network == NULL) {
					test_network = new Network();
				} else {
					test_network = new Network(this->existing_network);
				}
				test_network->increment((int)test_input_node_contexts.size());

				vector<vector<double>> test_inputs = inputs;
				for (int d_index = 0; d_index < num_instances; d_index++) {
					test_inputs[d_index].reserve((int)test_input_node_contexts.size());
					for (int t_index = (int)this->existing_input_node_contexts.size(); t_index < (int)test_input_node_contexts.size(); t_index++) {
						map<AbstractNode*, AbstractNodeHistory*>::iterator it = this->scope_histories[d_index]->node_histories.find(
							test_input_node_contexts[t_index]);
						if (it == this->scope_histories[d_index]->node_histories.end()) {
							test_inputs[d_index].push_back(0.0);
						} else {
							switch (it->first->type) {
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
							}
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
				if (this->existing_network == NULL) {
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

					int original_input_size = (int)this->existing_input_node_contexts.size();
					int test_input_size = (int)test_input_node_contexts.size();

					#if defined(MDEBUG) && MDEBUG
					#else
					average_misguess = test_average_misguess;
					misguess_standard_deviation = test_misguess_standard_deviation;
					#endif /* MDEBUG */

					this->existing_input_node_contexts = test_input_node_contexts;
					this->existing_input_obs_indexes = test_input_obs_indexes;

					if (this->existing_network != NULL) {
						delete this->existing_network;
					}
					this->existing_network = test_network;

					inputs = test_inputs;

					for (int i_index = test_input_size-1; i_index >= original_input_size; i_index--) {
						vector<AbstractNode*> remove_test_input_node_contexts = this->existing_input_node_contexts;
						vector<int> remove_test_input_obs_indexes = this->existing_input_obs_indexes;

						remove_test_input_node_contexts.erase(remove_test_input_node_contexts.begin() + i_index);
						remove_test_input_obs_indexes.erase(remove_test_input_obs_indexes.begin() + i_index);

						Network* remove_test_network = new Network(this->existing_network);
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
							this->existing_input_node_contexts = remove_test_input_node_contexts;
							this->existing_input_obs_indexes = remove_test_input_obs_indexes;

							delete this->existing_network;
							this->existing_network = remove_test_network;

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

		uniform_int_distribution<int> neutral_distribution(0, 9);
		if (neutral_distribution(generator) == 0) {
			this->explore_type = EXPLORE_TYPE_NEUTRAL;
		} else {
			uniform_int_distribution<int> best_distribution(0, 1);
			if (best_distribution(generator) == 0) {
				this->explore_type = EXPLORE_TYPE_BEST;

				this->best_surprise = 0.0;
			} else {
				this->explore_type = EXPLORE_TYPE_GOOD;
			}
		}

		this->state = NEW_INFO_EXPERIMENT_STATE_EXPLORE_SEQUENCE;
		this->state_iter = 0;
		this->explore_iter = 0;
	}
}
