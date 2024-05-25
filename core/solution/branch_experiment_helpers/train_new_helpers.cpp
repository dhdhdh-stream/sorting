// #include "branch_experiment.h"

// #include <cmath>
// #include <iostream>
// #undef eigen_assert
// #define eigen_assert(x) if (!(x)) {throw std::invalid_argument("Eigen error");}
// #include <Eigen/Dense>

// #include "action_node.h"
// #include "branch_node.h"
// #include "constants.h"
// #include "eval.h"
// #include "globals.h"
// #include "info_branch_node.h"
// #include "network.h"
// #include "nn_helpers.h"
// #include "scope.h"
// #include "scope_node.h"
// #include "solution.h"
// #include "solution_helpers.h"
// #include "utilities.h"

// using namespace std;

// void BranchExperiment::train_new_activate(
// 		AbstractNode*& curr_node,
// 		vector<ContextLayer>& context,
// 		RunHelper& run_helper,
// 		BranchExperimentHistory* history) {
// 	run_helper.num_decisions++;

// 	if (context.back().scope_history == run_helper.experiment_scope_history) {
// 		this->scope_histories.push_back(new ScopeHistory(context.back().scope_history));

// 		run_helper.num_actions_limit = MAX_NUM_ACTIONS_LIMIT_MULTIPLIER * solution->explore_scope_max_num_actions;

// 		if (this->best_step_types.size() == 0) {
// 			curr_node = this->best_exit_next_node;
// 		} else {
// 			if (this->best_step_types[0] == STEP_TYPE_ACTION) {
// 				curr_node = this->best_actions[0];
// 			} else {
// 				curr_node = this->best_scopes[0];
// 			}
// 		}
// 	}
// }

// void BranchExperiment::train_new_backprop(
// 		EvalHistory* eval_history,
// 		Problem* problem,
// 		vector<ContextLayer>& context,
// 		RunHelper& run_helper) {
// 	if (run_helper.num_actions_limit == 0) {
// 		run_helper.num_actions_limit = -1;

// 		this->explore_iter++;
// 		if (this->explore_iter < MAX_EXPLORE_TRIES) {
// 			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
// 				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
// 					delete this->best_actions[s_index];
// 				} else {
// 					delete this->best_scopes[s_index];
// 				}
// 			}

// 			this->best_step_types.clear();
// 			this->best_actions.clear();
// 			this->best_scopes.clear();

// 			if (this->ending_node != NULL) {
// 				delete this->ending_node;
// 				this->ending_node = NULL;
// 			}

// 			this->new_linear_weights.clear();
// 			this->new_network_input_indexes.clear();
// 			if (this->new_network != NULL) {
// 				delete this->new_network;
// 				this->new_network = NULL;
// 			}

// 			uniform_int_distribution<int> neutral_distribution(0, 9);
// 			if (neutral_distribution(generator) == 0) {
// 				this->explore_type = EXPLORE_TYPE_NEUTRAL;
// 			} else {
// 				uniform_int_distribution<int> best_distribution(0, 1);
// 				if (best_distribution(generator) == 0) {
// 					this->explore_type = EXPLORE_TYPE_BEST;

// 					this->best_surprise = 0.0;
// 				} else {
// 					this->explore_type = EXPLORE_TYPE_GOOD;
// 				}
// 			}

// 			this->state = BRANCH_EXPERIMENT_STATE_EXPLORE;
// 			this->state_iter = 0;
// 		} else {
// 			this->result = EXPERIMENT_RESULT_FAIL;
// 		}
// 	} else {
// 		this->scope_context->eval->activate(problem,
// 											run_helper,
// 											eval_history->end_scope_history);
// 		double predicted_impact = this->scope_context->eval->calc_vs(
// 			run_helper,
// 			eval_history);
// 		this->target_val_histories.push_back(predicted_impact);

// 		run_helper.num_actions_limit = -1;

// 		if ((int)this->target_val_histories.size() >= NUM_DATAPOINTS) {
// 			double sum_scores = 0.0;
// 			for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
// 				sum_scores += this->target_val_histories[d_index];
// 			}
// 			this->new_average_score = sum_scores / NUM_DATAPOINTS;

// 			Eigen::MatrixXd inputs(NUM_DATAPOINTS, this->input_scope_contexts.size());

// 			for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
// 				for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
// 					int curr_layer = 0;
// 					ScopeHistory* curr_scope_history = this->scope_histories[d_index];
// 					while (true) {
// 						map<AbstractNode*, AbstractNodeHistory*>::iterator it = curr_scope_history->node_histories.find(
// 							this->input_node_contexts[i_index][curr_layer]);
// 						if (it == curr_scope_history->node_histories.end()) {
// 							inputs(d_index, i_index) = 0.0;
// 							break;
// 						} else {
// 							if (curr_layer == (int)this->input_scope_contexts[i_index].size()-1) {
// 								switch (it->first->type) {
// 								case NODE_TYPE_ACTION:
// 									{
// 										ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
// 										inputs(d_index, i_index) = action_node_history->obs_snapshot[this->input_obs_indexes[i_index]];
// 									}
// 									break;
// 								case NODE_TYPE_BRANCH:
// 									{
// 										BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
// 										if (branch_node_history->is_branch) {
// 											inputs(d_index, i_index) = 1.0;
// 										} else {
// 											inputs(d_index, i_index) = -1.0;
// 										}
// 									}
// 									break;
// 								case NODE_TYPE_INFO_BRANCH:
// 									{
// 										InfoBranchNodeHistory* info_branch_node_history = (InfoBranchNodeHistory*)it->second;
// 										if (info_branch_node_history->is_branch) {
// 											inputs(d_index, i_index) = 1.0;
// 										} else {
// 											inputs(d_index, i_index) = -1.0;
// 										}
// 									}
// 									break;
// 								}
// 								break;
// 							} else {
// 								curr_layer++;
// 								curr_scope_history = ((ScopeNodeHistory*)it->second)->scope_history;
// 							}
// 						}
// 					}
// 				}
// 			}

// 			Eigen::VectorXd outputs(NUM_DATAPOINTS);
// 			for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
// 				outputs(d_index) = this->target_val_histories[d_index] - this->new_average_score;
// 			}

// 			Eigen::VectorXd weights;
// 			try {
// 				weights = inputs.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(outputs);
// 			} catch (std::invalid_argument &e) {
// 				cout << "Eigen error" << endl;
// 				weights = Eigen::VectorXd(this->input_scope_contexts.size());
// 				for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
// 					weights(i_index) = 0.0;
// 				}
// 			}
// 			this->new_linear_weights = vector<double>(this->input_scope_contexts.size());
// 			for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
// 				double sum_impact_size = 0.0;
// 				for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
// 					sum_impact_size += abs(inputs(d_index, i_index));
// 				}
// 				double average_impact = sum_impact_size / NUM_DATAPOINTS;
// 				if (abs(weights(i_index)) * average_impact < WEIGHT_MIN_SCORE_IMPACT * this->existing_score_standard_deviation
// 						|| abs(weights(i_index)) > LINEAR_MAX_WEIGHT) {
// 					weights(i_index) = 0.0;
// 				} else {
// 					weights(i_index) = trunc(1000000 * weights(i_index)) / 1000000;
// 				}
// 				this->new_linear_weights[i_index] = weights(i_index);
// 			}

// 			Eigen::VectorXd predicted_scores = inputs * weights;
// 			Eigen::VectorXd diffs = outputs - predicted_scores;
// 			vector<double> network_target_vals(NUM_DATAPOINTS);
// 			for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
// 				network_target_vals[d_index] = diffs(d_index);
// 			}

// 			vector<double> misguesses(NUM_DATAPOINTS);
// 			for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
// 				misguesses[d_index] = diffs(d_index) * diffs(d_index);
// 			}

// 			double sum_misguesses = 0.0;
// 			for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
// 				sum_misguesses += misguesses[d_index];
// 			}
// 			this->new_average_misguess = sum_misguesses / NUM_DATAPOINTS;

// 			double sum_misguess_variances = 0.0;
// 			for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
// 				sum_misguess_variances += (misguesses[d_index] - this->new_average_misguess) * (misguesses[d_index] - this->new_average_misguess);
// 			}
// 			this->new_misguess_standard_deviation = sqrt(sum_misguess_variances / NUM_DATAPOINTS);
// 			if (this->new_misguess_standard_deviation < MIN_STANDARD_DEVIATION) {
// 				this->new_misguess_standard_deviation = MIN_STANDARD_DEVIATION;
// 			}

// 			vector<vector<vector<double>>> network_inputs(NUM_DATAPOINTS);
// 			int train_index = 0;
// 			while (train_index < 3) {
// 				vector<vector<Scope*>> possible_scope_contexts;
// 				vector<vector<AbstractNode*>> possible_node_contexts;
// 				vector<int> possible_obs_indexes;

// 				vector<Scope*> scope_context;
// 				vector<AbstractNode*> node_context;
// 				uniform_int_distribution<int> history_distribution(0, NUM_DATAPOINTS-1);
// 				gather_possible_helper(scope_context,
// 									   node_context,
// 									   possible_scope_contexts,
// 									   possible_node_contexts,
// 									   possible_obs_indexes,
// 									   this->scope_histories[history_distribution(generator)]);

// 				int num_new_input_indexes = min(NETWORK_INCREMENT_NUM_NEW, (int)possible_scope_contexts.size());
// 				vector<vector<Scope*>> test_network_input_scope_contexts;
// 				vector<vector<AbstractNode*>> test_network_input_node_contexts;
// 				vector<int> test_network_input_obs_indexes;

// 				vector<int> remaining_indexes(possible_scope_contexts.size());
// 				for (int p_index = 0; p_index < (int)possible_scope_contexts.size(); p_index++) {
// 					remaining_indexes[p_index] = p_index;
// 				}
// 				for (int i_index = 0; i_index < num_new_input_indexes; i_index++) {
// 					uniform_int_distribution<int> distribution(0, (int)remaining_indexes.size()-1);
// 					int rand_index = distribution(generator);

// 					test_network_input_scope_contexts.push_back(possible_scope_contexts[remaining_indexes[rand_index]]);
// 					test_network_input_node_contexts.push_back(possible_node_contexts[remaining_indexes[rand_index]]);
// 					test_network_input_obs_indexes.push_back(possible_obs_indexes[remaining_indexes[rand_index]]);

// 					remaining_indexes.erase(remaining_indexes.begin() + rand_index);
// 				}

// 				Network* test_network;
// 				if (this->new_network == NULL) {
// 					test_network = new Network(num_new_input_indexes);
// 				} else {
// 					test_network = new Network(this->new_network);

// 					uniform_int_distribution<int> increment_above_distribution(0, 3);
// 					if (increment_above_distribution(generator) == 0) {
// 						test_network->increment_above(num_new_input_indexes);
// 					} else {
// 						test_network->increment_side(num_new_input_indexes);
// 					}
// 				}

// 				for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
// 					vector<double> test_input_vals(num_new_input_indexes, 0.0);
// 					for (int t_index = 0; t_index < num_new_input_indexes; t_index++) {
// 						int curr_layer = 0;
// 						ScopeHistory* curr_scope_history = this->scope_histories[d_index];
// 						while (true) {
// 							map<AbstractNode*, AbstractNodeHistory*>::iterator it = curr_scope_history->node_histories.find(
// 								test_network_input_node_contexts[t_index][curr_layer]);
// 							if (it == curr_scope_history->node_histories.end()) {
// 								break;
// 							} else {
// 								if (curr_layer == (int)test_network_input_scope_contexts[t_index].size()-1) {
// 									switch (it->first->type) {
// 									case NODE_TYPE_ACTION:
// 										{
// 											ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
// 											test_input_vals[t_index] = action_node_history->obs_snapshot[test_network_input_obs_indexes[t_index]];
// 										}
// 										break;
// 									case NODE_TYPE_BRANCH:
// 										{
// 											BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
// 											if (branch_node_history->is_branch) {
// 												test_input_vals[t_index] = 1.0;
// 											} else {
// 												test_input_vals[t_index] = -1.0;
// 											}
// 										}
// 										break;
// 									case NODE_TYPE_INFO_BRANCH:
// 										{
// 											InfoBranchNodeHistory* info_branch_node_history = (InfoBranchNodeHistory*)it->second;
// 											if (info_branch_node_history->is_branch) {
// 												test_input_vals[t_index] = 1.0;
// 											} else {
// 												test_input_vals[t_index] = -1.0;
// 											}
// 										}
// 										break;
// 									}
// 									break;
// 								} else {
// 									curr_layer++;
// 									curr_scope_history = ((ScopeNodeHistory*)it->second)->scope_history;
// 								}
// 							}
// 						}
// 					}
// 					network_inputs[d_index].push_back(test_input_vals);
// 				}

// 				train_network(network_inputs,
// 							  network_target_vals,
// 							  test_network_input_scope_contexts,
// 							  test_network_input_node_contexts,
// 							  test_network_input_obs_indexes,
// 							  test_network);

// 				double average_misguess;
// 				double misguess_standard_deviation;
// 				measure_network(network_inputs,
// 								network_target_vals,
// 								test_network,
// 								average_misguess,
// 								misguess_standard_deviation);

// 				#if defined(MDEBUG) && MDEBUG
// 				if (rand()%3 == 0) {
// 				#else
// 				double improvement = this->new_average_misguess - average_misguess;
// 				double standard_deviation = min(this->new_misguess_standard_deviation, misguess_standard_deviation);
// 				double t_score = improvement / (standard_deviation / sqrt(NUM_DATAPOINTS * TEST_SAMPLES_PERCENTAGE));

// 				if (t_score > 1.645) {
// 				#endif /* MDEBUG */
// 					optimize_network(network_inputs,
// 									 network_target_vals,
// 									 test_network);

// 					measure_network(network_inputs,
// 									network_target_vals,
// 									test_network,
// 									average_misguess,
// 									misguess_standard_deviation);

// 					vector<int> new_input_indexes;
// 					for (int t_index = 0; t_index < (int)test_network_input_scope_contexts.size(); t_index++) {
// 						int index = -1;
// 						for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
// 							if (test_network_input_scope_contexts[t_index] == this->input_scope_contexts[i_index]
// 									&& test_network_input_node_contexts[t_index] == this->input_node_contexts[i_index]
// 									&& test_network_input_obs_indexes[t_index] == this->input_obs_indexes[i_index]) {
// 								index = i_index;
// 								break;
// 							}
// 						}
// 						if (index == -1) {
// 							this->input_scope_contexts.push_back(test_network_input_scope_contexts[t_index]);
// 							this->input_node_contexts.push_back(test_network_input_node_contexts[t_index]);
// 							this->input_obs_indexes.push_back(test_network_input_obs_indexes[t_index]);

// 							this->existing_linear_weights.push_back(0.0);
// 							this->new_linear_weights.push_back(0.0);

// 							index = this->input_scope_contexts.size()-1;
// 						}
// 						new_input_indexes.push_back(index);
// 					}
// 					this->new_network_input_indexes.push_back(new_input_indexes);

// 					if (this->new_network != NULL) {
// 						delete this->new_network;
// 					}
// 					this->new_network = test_network;

// 					this->new_average_misguess = average_misguess;
// 					this->new_misguess_standard_deviation = misguess_standard_deviation;

// 					#if defined(MDEBUG) && MDEBUG
// 					#else
// 					train_index = 0;
// 					#endif /* MDEBUG */
// 				} else {
// 					delete test_network;

// 					for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
// 						network_inputs[d_index].pop_back();
// 					}

// 					train_index++;
// 				}
// 			}

// 			for (int i_index = 0; i_index < (int)this->scope_histories.size(); i_index++) {
// 				delete this->scope_histories[i_index];
// 			}
// 			this->scope_histories.clear();
// 			this->target_val_histories.clear();

// 			this->combined_score = 0.0;
// 			this->original_count = 0;
// 			this->branch_count = 0;

// 			this->state = BRANCH_EXPERIMENT_STATE_MEASURE;
// 			this->state_iter = 0;
// 		}
// 	}
// }
