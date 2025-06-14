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
#include "new_scope_experiment.h"
#include "nn_helpers.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

const int TRAIN_NEW_NUM_DATAPOINTS = 100;

void BranchExperiment::train_new_check_activate(
		SolutionWrapper* wrapper,
		BranchExperimentHistory* history) {
	this->num_instances_until_target--;

	if (this->num_instances_until_target <= 0) {
		history->instance_count++;

		double sum_vals = this->existing_average_score;
		for (int f_index = 0; f_index < (int)this->existing_factor_ids.size(); f_index++) {
			double val;
			fetch_factor_helper(wrapper->scope_histories.back(),
								this->existing_factor_ids[f_index],
								val);
			sum_vals += this->existing_factor_weights[f_index] * val;
		}
		history->existing_predicted_scores.push_back(sum_vals);

		vector<double> input_vals(this->new_inputs.size());
		for (int i_index = 0; i_index < (int)this->new_inputs.size(); i_index++) {
			fetch_input_helper(wrapper->scope_histories.back(),
							   this->new_inputs[i_index],
							   0,
							   input_vals[i_index]);
		}
		this->input_histories.push_back(input_vals);

		vector<double> factor_vals(this->new_factor_ids.size());
		for (int f_index = 0; f_index < (int)this->new_factor_ids.size(); f_index++) {
			fetch_factor_helper(wrapper->scope_histories.back(),
								this->new_factor_ids[f_index],
								factor_vals[f_index]);
		}
		this->factor_histories.push_back(factor_vals);

		uniform_int_distribution<int> until_distribution(0, 2*((int)this->average_instances_per_run-1));
		this->num_instances_until_target = 1 + until_distribution(generator);

		BranchExperimentState* new_experiment_state = new BranchExperimentState(this);
		new_experiment_state->step_index = 0;
		wrapper->experiment_context.back() = new_experiment_state;
	}
}

void BranchExperiment::train_new_step(vector<double>& obs,
									  string& action,
									  bool& is_next,
									  SolutionWrapper* wrapper,
									  BranchExperimentState* experiment_state) {
	if (experiment_state->step_index >= (int)this->best_step_types.size()) {
		wrapper->node_context.back() = this->best_exit_next_node;

		delete experiment_state;
		wrapper->experiment_context.back() = NULL;
	} else {
		if (this->best_step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			action = this->best_actions[experiment_state->step_index];
			is_next = true;

			wrapper->num_actions++;

			experiment_state->step_index++;
		} else {
			ScopeHistory* inner_scope_history = new ScopeHistory(this->best_scopes[experiment_state->step_index]);
			wrapper->scope_histories.push_back(inner_scope_history);
			wrapper->node_context.push_back(this->best_scopes[experiment_state->step_index]->nodes[0]);
			wrapper->experiment_context.push_back(NULL);
			wrapper->confusion_context.push_back(NULL);

			if (this->best_scopes[experiment_state->step_index]->new_scope_experiment != NULL) {
				this->best_scopes[experiment_state->step_index]->new_scope_experiment->pre_activate(wrapper);
			}
		}
	}
}

void BranchExperiment::train_new_exit_step(SolutionWrapper* wrapper,
										   BranchExperimentState* experiment_state) {
	if (this->best_scopes[experiment_state->step_index]->new_scope_experiment != NULL) {
		this->best_scopes[experiment_state->step_index]->new_scope_experiment->back_activate(wrapper);
	}

	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();
	wrapper->confusion_context.pop_back();

	experiment_state->step_index++;
}

void BranchExperiment::train_new_backprop(
		double target_val,
		BranchExperimentHistory* history) {
	for (int i_index = 0; i_index < history->instance_count; i_index++) {
		this->i_target_val_histories.push_back(target_val - history->existing_predicted_scores[i_index]);
	}

	this->state_iter++;
	if (this->state_iter >= TRAIN_NEW_NUM_DATAPOINTS) {
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
			shuffle(this->i_target_val_histories.begin(), this->i_target_val_histories.end(), generator_copy);
		}

		int num_instances = (int)this->i_target_val_histories.size();
		int num_train_instances = (double)num_instances * (1.0 - TEST_SAMPLES_PERCENTAGE);
		int num_test_instances = num_instances - num_train_instances;

		double sum_score = 0.0;
		for (int i_index = 0; i_index < num_instances; i_index++) {
			sum_score += this->i_target_val_histories[i_index];
		}
		this->new_average_score = sum_score / num_instances;

		vector<double> remaining_scores(num_instances);
		vector<double> sum_vals(num_instances);

		if (this->new_factor_ids.size() > 0) {
			double sum_offset = 0.0;
			for (int i_index = 0; i_index < num_train_instances; i_index++) {
				sum_offset += abs(this->i_target_val_histories[i_index] - this->new_average_score);
			}
			double average_offset = sum_offset / num_train_instances;

			Eigen::MatrixXd inputs(num_train_instances, this->new_factor_ids.size());
			for (int i_index = 0; i_index < num_train_instances; i_index++) {
				for (int f_index = 0; f_index < (int)this->new_factor_ids.size(); f_index++) {
					inputs(i_index, f_index) = this->factor_histories[i_index][f_index];
				}
			}

			Eigen::VectorXd outputs(num_train_instances);
			for (int i_index = 0; i_index < num_train_instances; i_index++) {
				outputs(i_index) = this->i_target_val_histories[i_index] - this->new_average_score;
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

			double impact_threshold = average_offset * FACTOR_IMPACT_THRESHOLD;

			for (int f_index = (int)this->new_factor_ids.size() - 1; f_index >= 0; f_index--) {
				double sum_impact = 0.0;
				for (int i_index = 0; i_index < num_train_instances; i_index++) {
					sum_impact += abs(this->factor_histories[i_index][f_index]);
				}

				double impact = abs(this->new_factor_weights[f_index]) * sum_impact
					/ num_train_instances;
				if (impact < impact_threshold
						|| abs(this->new_factor_weights[f_index]) > REGRESSION_WEIGHT_LIMIT) {
					this->new_factor_ids.erase(this->new_factor_ids.begin() + f_index);
					this->new_factor_weights.erase(this->new_factor_weights.begin() + f_index);

					for (int i_index = 0; i_index < num_instances; i_index++) {
						this->factor_histories[i_index].erase(this->factor_histories[i_index].begin() + f_index);
					}
				}
			}

			for (int i_index = 0; i_index < num_instances; i_index++) {
				double sum_score = 0.0;
				for (int f_index = 0; f_index < (int)this->new_factor_ids.size(); f_index++) {
					sum_score += this->new_factor_weights[f_index]
						* this->factor_histories[i_index][f_index];
				}

				if (abs(sum_score) > REGRESSION_FAIL_MULTIPLIER * average_offset) {
					this->result = EXPERIMENT_RESULT_FAIL;
					return;
				}

				remaining_scores[i_index] = this->i_target_val_histories[i_index]
					- this->new_average_score - sum_score;
				sum_vals[i_index] = this->new_average_score + sum_score;
			}
		} else {
			for (int i_index = 0; i_index < num_instances; i_index++) {
				remaining_scores[i_index] = this->i_target_val_histories[i_index] - this->new_average_score;
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
										   this->input_histories);

		train_network(this->input_histories,
					  remaining_scores,
					  new_network);

		double new_average_misguess;
		double new_misguess_standard_deviation;
		measure_network(this->input_histories,
						remaining_scores,
						new_network,
						new_average_misguess,
						new_misguess_standard_deviation);

		double new_improvement = average_misguess - new_average_misguess;
		double new_standard_deviation = min(misguess_standard_deviation, new_misguess_standard_deviation);
		double new_t_score = new_improvement / (new_standard_deviation / sqrt(num_test_instances));

		if (new_t_score > 2.326) {
			average_misguess = new_average_misguess;

			for (int i_index = (int)this->new_inputs.size()-1; i_index >= 0; i_index--) {
				vector<Input> remove_inputs = this->new_inputs;
				remove_inputs.erase(remove_inputs.begin() + i_index);

				Network* remove_network = new Network(new_network);
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

				double remove_improvement = average_misguess - remove_average_misguess;
				double remove_standard_deviation = min(misguess_standard_deviation, remove_misguess_standard_deviation);
				double remove_t_score = remove_improvement / (remove_standard_deviation / sqrt(num_instances * TEST_SAMPLES_PERCENTAGE));

				if (remove_t_score > -0.674) {
					this->new_inputs = remove_inputs;

					delete new_network;
					new_network = remove_network;

					this->input_histories = remove_input_vals;
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

					this->node_context->experiment = NULL;

					this->node_context = new_obs_node;
					this->is_branch = false;
					this->node_context->experiment = this;

					this->new_factor_ids.push_back({new_obs_node->id, 0});
					this->new_factor_weights.push_back(1.0);
				}

				for (int i_index = 0; i_index < num_instances; i_index++) {
					new_network->activate(this->input_histories[i_index]);
					sum_vals[i_index] += new_network->output->acti_vals[0];
				}
			} else {
				delete new_network;
			}
		} else {
			delete new_network;
		}

		int num_positive = 0;
		for (int i_index = 0; i_index < num_instances; i_index++) {
			if (sum_vals[i_index] > 0.0) {
				num_positive++;
			}
		}
		this->select_percentage = (double)num_positive / (double)num_instances;

		if (this->select_percentage > 0.0) {
			this->combined_score = 0.0;

			this->state = BRANCH_EXPERIMENT_STATE_MEASURE;
			this->state_iter = 0;
		} else {
			this->result = EXPERIMENT_RESULT_FAIL;
		}
	}
}
