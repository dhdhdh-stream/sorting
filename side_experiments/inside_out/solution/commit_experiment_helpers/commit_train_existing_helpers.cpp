#include "commit_experiment.h"

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
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_EXISTING_NUM_DATAPOINTS = 20;
#else
const int TRAIN_EXISTING_NUM_DATAPOINTS = 100;
#endif /* MDEBUG */

void CommitExperiment::commit_train_existing_check_activate(
		SolutionWrapper* wrapper) {
	CommitExperimentState* new_experiment_state = new CommitExperimentState(this);
	new_experiment_state->is_save = false;
	new_experiment_state->step_index = 0;
	wrapper->experiment_context.back() = new_experiment_state;
}

void CommitExperiment::commit_train_existing_step(
		vector<double>& obs,
		int& action,
		bool& is_next,
		SolutionWrapper* wrapper,
		CommitExperimentState* experiment_state) {
	if (experiment_state->is_save) {
		if (experiment_state->step_index >= (int)this->save_step_types.size()) {
			wrapper->node_context.back() = this->save_exit_next_node;

			delete experiment_state;
			wrapper->experiment_context.back() = NULL;
		} else {
			if (this->save_step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
				action = this->save_actions[experiment_state->step_index];
				is_next = true;

				wrapper->num_actions++;

				experiment_state->step_index++;
			} else {
				ScopeHistory* inner_scope_history = new ScopeHistory(this->save_scopes[experiment_state->step_index]);
				wrapper->scope_histories.push_back(inner_scope_history);
				wrapper->node_context.push_back(this->save_scopes[experiment_state->step_index]->nodes[0]);
				wrapper->experiment_context.push_back(NULL);
				wrapper->confusion_context.push_back(NULL);

				if (this->save_scopes[experiment_state->step_index]->new_scope_experiment != NULL) {
					this->save_scopes[experiment_state->step_index]->new_scope_experiment->pre_activate(wrapper);
				}
			}
		}
	} else {
		if (experiment_state->step_index == this->step_iter) {
			this->num_instances_until_target--;
			if (this->num_instances_until_target <= 0) {
				NewScopeExperimentHistory* history = (NewScopeExperimentHistory*)wrapper->experiment_history;
				history->instance_count++;

				vector<double> input_vals(this->commit_existing_inputs.size());
				for (int i_index = 0; i_index < (int)this->commit_existing_inputs.size(); i_index++) {
					fetch_input_helper(wrapper->scope_histories.back(),
									   this->commit_existing_inputs[i_index],
									   0,
									   input_vals[i_index]);
				}
				this->input_histories.push_back(input_vals);

				vector<double> factor_vals(this->commit_existing_factor_ids.size());
				for (int f_index = 0; f_index < (int)this->commit_existing_factor_ids.size(); f_index++) {
					fetch_factor_helper(wrapper->scope_histories.back(),
										this->commit_existing_factor_ids[f_index],
										factor_vals[f_index]);
				}
				this->factor_histories.push_back(factor_vals);
			} else {
				experiment_state->is_save = true;
				experiment_state->step_index = 0;
				return;
			}
		}

		if (experiment_state->step_index >= (int)this->new_nodes.size()) {
			wrapper->node_context.back() = this->best_exit_next_node;

			delete experiment_state;
			wrapper->experiment_context.back() = NULL;
		} else {
			switch (this->new_nodes[experiment_state->step_index]->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNode* node = (ActionNode*)this->new_nodes[experiment_state->step_index];

					action = node->action;
					is_next = true;

					wrapper->num_actions++;

					experiment_state->step_index++;
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* node = (ScopeNode*)this->new_nodes[experiment_state->step_index];

					ScopeHistory* scope_history = wrapper->scope_histories.back();

					ScopeNodeHistory* history = new ScopeNodeHistory(node);
					history->index = (int)scope_history->node_histories.size();
					scope_history->node_histories[node->id] = history;

					ScopeHistory* inner_scope_history = new ScopeHistory(node->scope);
					history->scope_history = inner_scope_history;
					wrapper->scope_histories.push_back(inner_scope_history);
					wrapper->node_context.push_back(node->scope->nodes[0]);
					wrapper->experiment_context.push_back(NULL);
					wrapper->confusion_context.push_back(NULL);

					if (node->scope->new_scope_experiment != NULL) {
						node->scope->new_scope_experiment->pre_activate(wrapper);
					}
				}
				break;
			case NODE_TYPE_OBS:
				{
					ObsNode* node = (ObsNode*)this->new_nodes[experiment_state->step_index];

					ScopeHistory* scope_history = wrapper->scope_histories.back();

					ObsNodeHistory* history = new ObsNodeHistory(node);
					history->index = (int)scope_history->node_histories.size();
					scope_history->node_histories[node->id] = history;

					history->obs_history = obs;

					history->factor_initialized = vector<bool>(node->factors.size(), false);
					history->factor_values = vector<double>(node->factors.size());

					experiment_state->step_index++;
				}
				break;
			}
		}
	}
}

void CommitExperiment::commit_train_existing_exit_step(
		SolutionWrapper* wrapper,
		CommitExperimentState* experiment_state) {
	if (experiment_state->is_save) {
		if (this->save_scopes[experiment_state->step_index]->new_scope_experiment != NULL) {
			this->save_scopes[experiment_state->step_index]->new_scope_experiment->back_activate(wrapper);
		}

		delete wrapper->scope_histories.back();

		wrapper->scope_histories.pop_back();
		wrapper->node_context.pop_back();
		wrapper->experiment_context.pop_back();
		wrapper->confusion_context.pop_back();

		experiment_state->step_index++;
	} else {
		ScopeNode* node = (ScopeNode*)this->new_nodes[experiment_state->step_index];

		if (node->scope->new_scope_experiment != NULL) {
			node->scope->new_scope_experiment->back_activate(wrapper);
		}

		wrapper->scope_histories.pop_back();
		wrapper->node_context.pop_back();
		wrapper->experiment_context.pop_back();
		wrapper->confusion_context.pop_back();

		experiment_state->step_index++;
	}
}

void CommitExperiment::commit_train_existing_backprop(
		double target_val,
		CommitExperimentHistory* history) {
	for (int i_index = 0; i_index < history->instance_count; i_index++) {
		this->i_target_val_histories.push_back(target_val);
	}

	this->state_iter++;
	if (this->state_iter >= TRAIN_EXISTING_NUM_DATAPOINTS) {
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
		this->commit_existing_average_score = sum_score / num_instances;

		vector<double> remaining_scores(num_instances);

		if (this->commit_existing_factor_ids.size() > 0) {
			#if defined(MDEBUG) && MDEBUG
			#else
			double sum_offset = 0.0;
			for (int i_index = 0; i_index < num_train_instances; i_index++) {
				sum_offset += abs(this->i_target_val_histories[i_index] - this->commit_existing_average_score);
			}
			double average_offset = sum_offset / num_train_instances;
			#endif /* MDEBUG */

			Eigen::MatrixXd inputs(num_train_instances, this->commit_existing_factor_ids.size());
			for (int i_index = 0; i_index < num_train_instances; i_index++) {
				for (int f_index = 0; f_index < (int)this->commit_existing_factor_ids.size(); f_index++) {
					inputs(i_index, f_index) = this->factor_histories[i_index][f_index];
				}
			}

			Eigen::VectorXd outputs(num_train_instances);
			for (int i_index = 0; i_index < num_train_instances; i_index++) {
				outputs(i_index) = this->i_target_val_histories[i_index] - this->commit_existing_average_score;
			}

			Eigen::VectorXd weights;
			try {
				weights = inputs.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(outputs);
			} catch (std::invalid_argument &e) {
				cout << "Eigen error" << endl;
				weights = Eigen::VectorXd(this->commit_existing_factor_ids.size());
				for (int f_index = 0; f_index < (int)this->commit_existing_factor_ids.size(); f_index++) {
					weights(f_index) = 0.0;
				}
			}

			for (int f_index = 0; f_index < (int)this->commit_existing_factor_ids.size(); f_index++) {
				this->commit_existing_factor_weights.push_back(weights(f_index));
			}

			#if defined(MDEBUG) && MDEBUG
			#else
			double impact_threshold = average_offset * FACTOR_IMPACT_THRESHOLD;
			#endif /* MDEBUG */

			for (int f_index = (int)this->commit_existing_factor_ids.size() - 1; f_index >= 0; f_index--) {
				#if defined(MDEBUG) && MDEBUG
				if (rand()%2 == 0) {
				#else
				double sum_impact = 0.0;
				for (int i_index = 0; i_index < num_train_instances; i_index++) {
					sum_impact += abs(this->factor_histories[i_index][f_index]);
				}

				double impact = abs(this->commit_existing_factor_weights[f_index]) * sum_impact
					/ num_train_instances;
				if (impact < impact_threshold
						|| abs(this->commit_existing_factor_weights[f_index]) > REGRESSION_WEIGHT_LIMIT) {
				#endif /* MDEBUG */
					this->commit_existing_factor_ids.erase(this->commit_existing_factor_ids.begin() + f_index);
					this->commit_existing_factor_weights.erase(this->commit_existing_factor_weights.begin() + f_index);

					for (int i_index = 0; i_index < num_instances; i_index++) {
						this->factor_histories[i_index].erase(this->factor_histories[i_index].begin() + f_index);
					}
				}
			}

			for (int i_index = 0; i_index < num_instances; i_index++) {
				double sum_score = 0.0;
				for (int f_index = 0; f_index < (int)this->commit_existing_factor_ids.size(); f_index++) {
					sum_score += this->commit_existing_factor_weights[f_index]
						* this->factor_histories[i_index][f_index];
				}

				#if defined(MDEBUG) && MDEBUG
				#else
				if (abs(sum_score) > REGRESSION_FAIL_MULTIPLIER * average_offset) {
					this->result = EXPERIMENT_RESULT_FAIL;
					return;
				}
				#endif /* MDEBUG */

				remaining_scores[i_index] = this->i_target_val_histories[i_index]
					- this->commit_existing_average_score - sum_score;
			}
		} else {
			for (int i_index = 0; i_index < num_instances; i_index++) {
				remaining_scores[i_index] = this->i_target_val_histories[i_index] - this->commit_existing_average_score;
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

		Network* existing_network = new Network((int)this->commit_existing_inputs.size(),
												this->input_histories);

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

		#if defined(MDEBUG) && MDEBUG
		if (rand()%2 == 0) {
		#else
		double new_improvement = average_misguess - new_average_misguess;
		double new_standard_deviation = min(misguess_standard_deviation, new_misguess_standard_deviation);
		double new_t_score = new_improvement / (new_standard_deviation / sqrt(num_test_instances));

		if (new_t_score > 2.326) {
		#endif /* MDEBUG */
			average_misguess = new_average_misguess;

			for (int i_index = (int)this->commit_existing_inputs.size()-1; i_index >= 0; i_index--) {
				vector<Input> remove_inputs = this->commit_existing_inputs;
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
					this->commit_existing_inputs = remove_inputs;

					delete existing_network;
					existing_network = remove_network;

					this->input_histories = remove_input_vals;
				} else {
					delete remove_network;
				}
			}

			if (this->commit_existing_inputs.size() > 0) {
				Factor* new_factor = new Factor();
				new_factor->inputs = this->commit_existing_inputs;
				new_factor->network = existing_network;

				ObsNode* obs_node = (ObsNode*)this->new_nodes[this->step_iter-1];

				obs_node->factors.push_back(new_factor);

				this->commit_existing_factor_ids.push_back({obs_node->id, (int)obs_node->factors.size()-1});
				this->commit_existing_factor_weights.push_back(1.0);
			} else {
				delete existing_network;
			}
		} else {
			delete existing_network;
		}

		this->input_histories.clear();
		this->factor_histories.clear();
		this->i_target_val_histories.clear();

		uniform_int_distribution<int> until_distribution(0, (int)this->average_instances_per_run-1.0);
		this->num_instances_until_target = 1 + until_distribution(generator);

		this->state = COMMIT_EXPERIMENT_STATE_COMMIT_NEW_GATHER;
		this->state_iter = 0;
	}
}
