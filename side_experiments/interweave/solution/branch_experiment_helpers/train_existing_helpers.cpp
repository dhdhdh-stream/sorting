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
#include "scope.h"
#include "scope_node.h"
#include "solution_helpers.h"

using namespace std;

void BranchExperiment::train_existing_activate(
		ScopeHistory* scope_history,
		BranchExperimentHistory* history) {
	history->instance_count++;

	vector<double> input_vals(this->existing_inputs.size());
	for (int i_index = 0; i_index < (int)this->existing_inputs.size(); i_index++) {
		fetch_input_helper(scope_history,
						   this->existing_inputs[i_index],
						   0,
						   input_vals[i_index]);
	}
	this->input_histories.push_back(input_vals);

	vector<double> factor_vals(this->existing_factor_ids.size());
	for (int f_index = 0; f_index < (int)this->existing_factor_ids.size(); f_index++) {
		fetch_factor_helper(scope_history,
							this->existing_factor_ids[f_index],
							factor_vals[f_index]);
	}
	this->factor_histories.push_back(factor_vals);
}

void BranchExperiment::train_existing_update(
		BranchExperimentHistory* history,
		double target_val) {
	for (int i_index = 0; i_index < history->instance_count; i_index++) {
		this->target_val_histories.push_back(target_val);
	}

	this->sum_num_instances += history->instance_count;

	this->run_iter++;
	if (this->run_iter >= TRAIN_NUM_DATAPOINTS) {
		this->average_instances_per_run = (double)this->sum_num_instances / (double)this->run_iter;
		if (this->average_instances_per_run < 1.0) {
			this->average_instances_per_run = 1.0;
		}

		{
			default_random_engine generator_copy = generator;
			shuffle(this->input_histories.begin(), this->input_histories.end(), generator_copy);
		}
		{
			default_random_engine generator_copy = generator;
			shuffle(this->factor_histories.begin(), this->factor_histories.end(), generator_copy);
		}
		{
			default_random_engine generator_copy = generator;
			shuffle(this->target_val_histories.begin(), this->target_val_histories.end(), generator_copy);
		}

		int num_instances = (int)this->target_val_histories.size();
		int num_train_instances = (double)num_instances * (1.0 - TEST_SAMPLES_PERCENTAGE);
		int num_test_instances = num_instances - num_train_instances;

		double sum_score = 0.0;
		for (int i_index = 0; i_index < num_instances; i_index++) {
			sum_score += this->target_val_histories[i_index];
		}
		this->existing_average_score = sum_score / num_instances;

		vector<double> remaining_scores(num_instances);

		if (this->existing_factor_ids.size() > 0) {
			double sum_offset = 0.0;
			for (int i_index = 0; i_index < num_train_instances; i_index++) {
				sum_offset += abs(this->target_val_histories[i_index] - this->existing_average_score);
			}
			double average_offset = sum_offset / num_train_instances;

			Eigen::MatrixXd inputs(num_train_instances, this->existing_factor_ids.size());
			for (int i_index = 0; i_index < num_train_instances; i_index++) {
				for (int f_index = 0; f_index < (int)this->existing_factor_ids.size(); f_index++) {
					inputs(i_index, f_index) = this->factor_histories[i_index][f_index];
				}
			}

			Eigen::VectorXd outputs(num_train_instances);
			for (int i_index = 0; i_index < num_train_instances; i_index++) {
				outputs(i_index) = this->target_val_histories[i_index] - this->existing_average_score;
			}

			Eigen::VectorXd weights;
			try {
				weights = inputs.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(outputs);
			} catch (std::invalid_argument &e) {
				cout << "Eigen error" << endl;
				weights = Eigen::VectorXd(this->existing_factor_ids.size());
				for (int f_index = 0; f_index < (int)this->existing_factor_ids.size(); f_index++) {
					weights(f_index) = 0.0;
				}
			}

			for (int f_index = 0; f_index < (int)this->existing_factor_ids.size(); f_index++) {
				this->existing_factor_weights.push_back(weights(f_index));
			}

			#if defined(MDEBUG) && MDEBUG
			#else
			double impact_threshold = average_offset * FACTOR_IMPACT_THRESHOLD;
			#endif /* MDEBUG */

			for (int f_index = (int)this->existing_factor_ids.size() - 1; f_index >= 0; f_index--) {
				#if defined(MDEBUG) && MDEBUG
				if (rand()%2 == 0) {
				#else
				double sum_impact = 0.0;
				for (int i_index = 0; i_index < num_train_instances; i_index++) {
					sum_impact += abs(this->factor_histories[i_index][f_index]);
				}

				double impact = abs(this->existing_factor_weights[f_index]) * sum_impact
					/ num_train_instances;
				if (impact < impact_threshold
						|| abs(this->existing_factor_weights[f_index]) > REGRESSION_WEIGHT_LIMIT) {
				#endif /* MDEBUG */
					this->existing_factor_ids.erase(this->existing_factor_ids.begin() + f_index);
					this->existing_factor_weights.erase(this->existing_factor_weights.begin() + f_index);

					for (int i_index = 0; i_index < num_instances; i_index++) {
						this->factor_histories[i_index].erase(this->factor_histories[i_index].begin() + f_index);
					}
				}
			}

			for (int i_index = 0; i_index < num_instances; i_index++) {
				double sum_score = this->existing_average_score;
				for (int f_index = 0; f_index < (int)this->existing_factor_ids.size(); f_index++) {
					sum_score += this->existing_factor_weights[f_index]
						* this->factor_histories[i_index][f_index];
				}

				remaining_scores[i_index] = this->target_val_histories[i_index] - sum_score;

				if (abs(sum_score) > REGRESSION_FAIL_MULTIPLIER * average_offset) {
					this->result = EXPERIMENT_RESULT_FAIL;
					return;
				}
			}
		} else {
			for (int i_index = 0; i_index < num_instances; i_index++) {
				remaining_scores[i_index] = this->target_val_histories[i_index] - this->existing_average_score;
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

		Network* existing_network = new Network((int)this->existing_inputs.size());

		train_network(this->input_histories,
					  remaining_scores,
					  existing_network);

		double new_average_misguess;
		double new_misguess_standard_deviation;
		measure_network(this->input_histories,
						remaining_scores,
						existing_network,
						new_average_misguess,
						new_misguess_standard_deviation);

		double new_improvement = average_misguess - new_average_misguess;
		double new_standard_deviation = min(misguess_standard_deviation, new_misguess_standard_deviation);
		double new_t_score = new_improvement / (new_standard_deviation / sqrt(num_test_instances));

		if (new_t_score > 2.326) {
			average_misguess = new_average_misguess;

			for (int i_index = (int)this->existing_inputs.size()-1; i_index >= 0; i_index--) {
				vector<pair<pair<vector<Scope*>,vector<int>>,pair<int,int>>> remove_inputs = this->existing_inputs;
				remove_inputs.erase(remove_inputs.begin() + i_index);

				Network* remove_network = new Network(existing_network);
				remove_network->remove_input(i_index);

				vector<vector<double>> remove_input_vals = this->input_histories;
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
					this->existing_inputs = remove_inputs;

					delete existing_network;
					existing_network = remove_network;

					this->input_histories = remove_input_vals;
				} else {
					delete remove_network;
				}
			}

			if (this->existing_inputs.size() > 0) {
				Factor* new_factor = new Factor();
				new_factor->inputs = this->existing_inputs;
				new_factor->network = existing_network;
				if (this->node_context->type == NODE_TYPE_OBS) {
					ObsNode* obs_node = (ObsNode*)this->node_context;

					obs_node->factors.push_back(new_factor);

					this->existing_factor_ids.push_back({obs_node->id, (int)obs_node->factors.size()-1});
					this->existing_factor_weights.push_back(1.0);
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

					this->node_context->experiment = NULL;

					this->node_context = new_obs_node;
					this->node_context->experiment = this;

					this->existing_factor_ids.push_back({new_obs_node->id, 0});
					this->existing_factor_weights.push_back(1.0);
				}
			} else {
				delete existing_network;
			}
		} else {
			delete existing_network;
		}

		this->input_histories.clear();
		this->factor_histories.clear();
		this->target_val_histories.clear();

		this->best_surprise = 0.0;

		uniform_int_distribution<int> until_distribution(0, (int)this->average_instances_per_run-1.0);
		this->num_instances_until_target = 1 + until_distribution(generator);

		this->state = BRANCH_EXPERIMENT_STATE_EXPLORE;
		this->run_iter = 0;
	}
}
