#include "branch_experiment.h"

#include <cmath>
#include <iostream>
#undef eigen_assert
#define eigen_assert(x) if (!(x)) {throw std::invalid_argument("Eigen error");}
#include <Eigen/Dense>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "factor.h"
#include "globals.h"
#include "network.h"
#include "nn_helpers.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_helpers.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_NEW_NUM_DATAPOINTS = 20;
#else
const int TRAIN_NEW_NUM_DATAPOINTS = 100;
#endif /* MDEBUG */

void BranchExperiment::train_new_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		RunHelper& run_helper,
		ScopeHistory* scope_history,
		BranchExperimentHistory* history) {
	if (history->is_active) {
		this->num_instances_until_target--;

		if (this->num_instances_until_target <= 0) {
			double sum_vals = this->existing_average_score;
			for (int f_index = 0; f_index < (int)this->existing_factor_ids.size(); f_index++) {
				double val;
				fetch_factor_helper(scope_history,
									this->existing_factor_ids[f_index],
									val);
				sum_vals += this->existing_factor_weights[f_index] * val;
			}
			history->existing_predicted_scores.push_back(sum_vals);

			vector<double> input_vals(this->new_inputs.size());
			for (int i_index = 0; i_index < (int)this->new_inputs.size(); i_index++) {
				fetch_input_helper(scope_history,
								   this->new_inputs[i_index],
								   0,
								   input_vals[i_index]);
			}
			this->new_input_histories.push_back(input_vals);

			vector<double> factor_vals(this->new_factor_ids.size());
			for (int f_index = 0; f_index < (int)this->new_factor_ids.size(); f_index++) {
				fetch_factor_helper(scope_history,
									this->new_factor_ids[f_index],
									factor_vals[f_index]);
			}
			this->new_factor_histories.push_back(factor_vals);

			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					problem->perform_action(this->best_actions[s_index]);

					run_helper.num_actions++;
				} else {
					ScopeHistory* inner_scope_history = new ScopeHistory(this->best_scopes[s_index]);
					this->best_scopes[s_index]->activate(problem,
						run_helper,
						inner_scope_history);
					delete inner_scope_history;
				}
			}

			curr_node = this->best_exit_next_node;

			uniform_int_distribution<int> until_distribution(0, 2*((int)this->average_instances_per_run-1));
			this->num_instances_until_target = 1 + until_distribution(generator);
		}
	}
}

void BranchExperiment::train_new_backprop(
		double target_val,
		BranchExperimentHistory* history) {
	if (history->is_active) {
		for (int i_index = 0; i_index < (int)history->existing_predicted_scores.size(); i_index++) {
			this->new_target_val_histories.push_back(target_val - history->existing_predicted_scores[i_index]);
		}

		this->state_iter++;
		if (this->state_iter >= TRAIN_NEW_NUM_DATAPOINTS) {
			{
				default_random_engine generator_copy = generator;
				shuffle(this->new_input_histories.begin(), this->new_input_histories.end(), generator_copy);
			}
			{
				default_random_engine generator_copy = generator;
				shuffle(this->new_factor_histories.begin(), this->new_factor_histories.end(), generator_copy);
			}
			{
				default_random_engine generator_copy = generator;
				shuffle(this->new_target_val_histories.begin(), this->new_target_val_histories.end(), generator_copy);
			}

			int num_instances = (int)this->new_target_val_histories.size();
			int num_train_instances = (double)num_instances * (1.0 - TEST_SAMPLES_PERCENTAGE);
			int num_test_instances = num_instances - num_train_instances;

			double sum_score = 0.0;
			for (int i_index = 0; i_index < num_instances; i_index++) {
				sum_score += this->new_target_val_histories[i_index];
			}
			this->new_average_score = sum_score / num_instances;

			vector<double> remaining_scores(num_instances);
			vector<double> sum_vals(num_instances);

			if (this->new_factor_ids.size() > 0) {
				#if defined(MDEBUG) && MDEBUG
				#else
				double sum_offset = 0.0;
				for (int i_index = 0; i_index < num_train_instances; i_index++) {
					sum_offset += abs(this->new_target_val_histories[i_index] - this->new_average_score);
				}
				double average_offset = sum_offset / num_train_instances;
				#endif /* MDEBUG */

				Eigen::MatrixXd inputs(num_train_instances, this->new_factor_ids.size());
				for (int i_index = 0; i_index < num_train_instances; i_index++) {
					for (int f_index = 0; f_index < (int)this->new_factor_ids.size(); f_index++) {
						inputs(i_index, f_index) = this->new_factor_histories[i_index][f_index];
					}
				}

				Eigen::VectorXd outputs(num_train_instances);
				for (int i_index = 0; i_index < num_train_instances; i_index++) {
					outputs(i_index) = this->new_target_val_histories[i_index] - this->new_average_score;
				}

				Eigen::VectorXd weights;
				try {
					weights = inputs.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(outputs);
				} catch (std::invalid_argument &e) {
					cout << "Eigen error" << endl;
					weights = Eigen::VectorXd(this->new_factor_ids.size());
					for (int f_index = 0; f_index < (int)this->new_factor_ids.size(); f_index++) {
						weights(f_index) = 0.0;
					}
				}

				for (int f_index = 0; f_index < (int)this->new_factor_ids.size(); f_index++) {
					this->new_factor_weights.push_back(weights(f_index));
				}

				#if defined(MDEBUG) && MDEBUG
				#else
				double impact_threshold = average_offset * FACTOR_IMPACT_THRESHOLD;
				#endif /* MDEBUG */

				for (int f_index = (int)this->new_factor_ids.size() - 1; f_index >= 0; f_index--) {
					#if defined(MDEBUG) && MDEBUG
					if (rand()%2 == 0) {
					#else
					double sum_impact = 0.0;
					for (int i_index = 0; i_index < num_train_instances; i_index++) {
						sum_impact += abs(this->new_factor_histories[i_index][f_index]);
					}

					double impact = abs(this->new_factor_weights[f_index]) * sum_impact
						/ num_train_instances;
					if (impact < impact_threshold
							|| abs(this->new_factor_weights[f_index]) > REGRESSION_WEIGHT_LIMIT) {
					#endif /* MDEBUG */
						this->new_factor_ids.erase(this->new_factor_ids.begin() + f_index);
						this->new_factor_weights.erase(this->new_factor_weights.begin() + f_index);

						for (int i_index = 0; i_index < num_instances; i_index++) {
							this->new_factor_histories[i_index].erase(this->new_factor_histories[i_index].begin() + f_index);
						}
					}
				}

				for (int i_index = 0; i_index < num_instances; i_index++) {
					double sum_score = 0.0;
					for (int f_index = 0; f_index < (int)this->new_factor_ids.size(); f_index++) {
						sum_score += this->new_factor_weights[f_index]
							* this->new_factor_histories[i_index][f_index];
					}

					#if defined(MDEBUG) && MDEBUG
					#else
					if (abs(sum_score) > REGRESSION_FAIL_MULTIPLIER * average_offset) {
						delete this;
						return;
					}
					#endif /* MDEBUG */

					remaining_scores[i_index] = this->new_target_val_histories[i_index]
						- this->new_average_score - sum_score;
					sum_vals[i_index] = this->new_average_score + sum_score;
				}
			} else {
				for (int i_index = 0; i_index < num_instances; i_index++) {
					remaining_scores[i_index] = this->new_target_val_histories[i_index] - this->new_average_score;
					sum_vals[i_index] = this->new_average_score;
				}
			}

			double sum_misguess = 0.0;
			for (int i_index = num_train_instances; i_index < num_instances; i_index++) {
				sum_misguess += remaining_scores[i_index] * remaining_scores[i_index];
			}
			double average_misguess = sum_misguess / num_test_instances;

			double sum_misguess_variance = 0.0;
			for (int i_index = num_train_instances; i_index < num_instances; i_index++) {
				double curr_misguess = remaining_scores[i_index] * remaining_scores[i_index];
				sum_misguess_variance += (curr_misguess - average_misguess) * (curr_misguess - average_misguess);
			}
			double misguess_standard_deviation = sqrt(sum_misguess_variance / num_test_instances);
			if (misguess_standard_deviation < MIN_STANDARD_DEVIATION) {
				misguess_standard_deviation = MIN_STANDARD_DEVIATION;
			}

			Network* new_network = new Network((int)this->new_inputs.size(),
											   this->new_input_histories);

			train_network(this->new_input_histories,
						  remaining_scores,
						  new_network);

			double new_average_misguess;
			double new_misguess_standard_deviation;
			measure_network(this->new_input_histories,
							remaining_scores,
							new_network,
							new_average_misguess,
							new_misguess_standard_deviation);

			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			double new_improvement = average_misguess - new_average_misguess;
			double new_standard_deviation = min(misguess_standard_deviation, new_misguess_standard_deviation);
			double new_t_score = new_improvement / (new_standard_deviation / sqrt(num_test_instances));

			if (new_t_score > 2.326) {
			#endif /* MDEBUG */
				average_misguess = new_average_misguess;

				for (int i_index = (int)this->new_inputs.size()-1; i_index >= 0; i_index--) {
					vector<Input> remove_inputs = this->new_inputs;
					remove_inputs.erase(remove_inputs.begin() + i_index);

					Network* remove_network = new Network(new_network);
					remove_network->remove_input(i_index);

					vector<vector<double>> remove_input_vals = this->new_input_histories;
					for (int d_index = 0; d_index < num_instances; d_index++) {
						remove_input_vals[d_index].erase(remove_input_vals[d_index].begin() + i_index);
					}

					optimize_network(remove_input_vals,
									 remaining_scores,
									 remove_network);

					double remove_average_misguess;
					double remove_misguess_standard_deviation;
					measure_network(remove_input_vals,
									remaining_scores,
									remove_network,
									remove_average_misguess,
									remove_misguess_standard_deviation);

					#if defined(MDEBUG) && MDEBUG
					if (rand()%2 == 0) {
					#else
					double remove_improvement = average_misguess - remove_average_misguess;
					double remove_standard_deviation = min(misguess_standard_deviation, remove_misguess_standard_deviation);
					double remove_t_score = remove_improvement / (remove_standard_deviation / sqrt(num_instances * TEST_SAMPLES_PERCENTAGE));

					if (remove_t_score > -0.674) {
					#endif /* MDEBUG */
						this->new_inputs = remove_inputs;

						delete new_network;
						new_network = remove_network;

						this->new_input_histories = remove_input_vals;
					} else {
						delete remove_network;
					}
				}

				if (this->new_inputs.size() > 0) {
					Factor* new_factor = new Factor();
					new_factor->inputs = this->new_inputs;
					new_factor->network = new_network;
					if (this->node_context->type == NODE_TYPE_OBS) {
						ObsNode* obs_node = (ObsNode*)this->node_context;

						obs_node->factors.push_back(new_factor);

						this->new_factor_ids.push_back({obs_node->id, (int)obs_node->factors.size()-1});
						this->new_factor_weights.push_back(1.0);
					} else {
						ObsNode* new_obs_node = new ObsNode();
						new_obs_node->parent = this->scope_context;
						new_obs_node->id = this->scope_context->node_counter;
						this->scope_context->node_counter++;
						this->scope_context->nodes[new_obs_node->id] = new_obs_node;

						new_obs_node->factors.push_back(new_factor);

						switch (this->node_context->type) {
						case NODE_TYPE_ACTION:
							{
								ActionNode* action_node = (ActionNode*)this->node_context;

								new_obs_node->next_node_id = action_node->next_node_id;
								new_obs_node->next_node = action_node->next_node;

								for (int a_index = 0; a_index < (int)action_node->next_node->ancestor_ids.size(); a_index++) {
									if (action_node->next_node->ancestor_ids[a_index] == action_node->id) {
										action_node->next_node->ancestor_ids.erase(
											action_node->next_node->ancestor_ids.begin() + a_index);
										break;
									}
								}
								action_node->next_node->ancestor_ids.push_back(new_obs_node->id);

								action_node->next_node_id = new_obs_node->id;
								action_node->next_node = new_obs_node;

								new_obs_node->ancestor_ids.push_back(action_node->id);
							}
							break;
						case NODE_TYPE_SCOPE:
							{
								ScopeNode* scope_node = (ScopeNode*)this->node_context;

								new_obs_node->next_node_id = scope_node->next_node_id;
								new_obs_node->next_node = scope_node->next_node;

								for (int a_index = 0; a_index < (int)scope_node->next_node->ancestor_ids.size(); a_index++) {
									if (scope_node->next_node->ancestor_ids[a_index] == scope_node->id) {
										scope_node->next_node->ancestor_ids.erase(
											scope_node->next_node->ancestor_ids.begin() + a_index);
										break;
									}
								}
								scope_node->next_node->ancestor_ids.push_back(new_obs_node->id);

								scope_node->next_node_id = new_obs_node->id;
								scope_node->next_node = new_obs_node;

								new_obs_node->ancestor_ids.push_back(scope_node->id);
							}
							break;
						case NODE_TYPE_BRANCH:
							{
								BranchNode* branch_node = (BranchNode*)this->node_context;

								if (this->is_branch) {
									new_obs_node->next_node_id = branch_node->branch_next_node_id;
									new_obs_node->next_node = branch_node->branch_next_node;

									for (int a_index = 0; a_index < (int)branch_node->branch_next_node->ancestor_ids.size(); a_index++) {
										if (branch_node->branch_next_node->ancestor_ids[a_index] == branch_node->id) {
											branch_node->branch_next_node->ancestor_ids.erase(
												branch_node->branch_next_node->ancestor_ids.begin() + a_index);
											break;
										}
									}
									branch_node->branch_next_node->ancestor_ids.push_back(new_obs_node->id);

									branch_node->branch_next_node_id = new_obs_node->id;
									branch_node->branch_next_node = new_obs_node;
								} else {
									new_obs_node->next_node_id = branch_node->original_next_node_id;
									new_obs_node->next_node = branch_node->original_next_node;

									for (int a_index = 0; a_index < (int)branch_node->original_next_node->ancestor_ids.size(); a_index++) {
										if (branch_node->original_next_node->ancestor_ids[a_index] == branch_node->id) {
											branch_node->original_next_node->ancestor_ids.erase(
												branch_node->original_next_node->ancestor_ids.begin() + a_index);
											break;
										}
									}
									branch_node->original_next_node->ancestor_ids.push_back(new_obs_node->id);

									branch_node->original_next_node_id = new_obs_node->id;
									branch_node->original_next_node = new_obs_node;
								}

								new_obs_node->ancestor_ids.push_back(branch_node->id);
							}
							break;
						}

						new_obs_node->is_init = true;

						this->node_context->experiment = NULL;

						this->node_context = new_obs_node;
						this->is_branch = false;
						this->node_context->experiment = this;

						this->new_factor_ids.push_back({new_obs_node->id, 0});
						this->new_factor_weights.push_back(1.0);
					}

					for (int i_index = 0; i_index < num_instances; i_index++) {
						new_network->activate(this->new_input_histories[i_index]);
						sum_vals[i_index] += new_network->output->acti_vals[0];
					}
				} else {
					delete new_network;
				}
			} else {
				delete new_network;
			}

			this->new_inputs.clear();

			this->new_input_histories.clear();
			this->new_factor_histories.clear();
			this->new_target_val_histories.clear();

			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
				this->select_percentage = 0.5;
			} else {
				this->select_percentage = 0.0;
			}
			#else
			int num_positive = 0;
			for (int i_index = 0; i_index < num_instances; i_index++) {
				if (sum_vals[i_index] > 0.0) {
					num_positive++;
				}
			}
			this->select_percentage = (double)num_positive / (double)num_instances;
			#endif /* MDEBUG */

			if (this->select_percentage > 0.0) {
				this->existing_sum_score = 0.0;
				this->existing_count = 0;
				this->combined_sum_score = 0.0;
				this->combined_count = 0;

				this->state = BRANCH_EXPERIMENT_STATE_MEASURE_1_PERCENT;
				this->state_iter = 0;
			} else {
				delete this;
			}
		}
	}
}
