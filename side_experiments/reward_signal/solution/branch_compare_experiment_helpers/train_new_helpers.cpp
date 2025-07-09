#include "branch_compare_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_NEW_NUM_DATAPOINTS = 20;
#else
const int TRAIN_NEW_NUM_DATAPOINTS = 100;
#endif /* MDEBUG */

void BranchCompareExperiment::train_new_check_activate(
		SolutionWrapper* wrapper,
		BranchCompareExperimentHistory* history) {
	this->num_instances_until_target--;

	if (this->num_instances_until_target <= 0) {
		double sum_vals = this->existing_signal_average_score;
		for (int i_index = 0; i_index < (int)this->existing_signal_inputs.size(); i_index++) {
			double val;
			bool is_on;
			fetch_input_helper(wrapper->scope_histories.back(),
							   this->existing_signal_inputs[i_index],
							   0,
							   val,
							   is_on);
			if (is_on) {
				double normalized_val = (val - this->existing_signal_input_averages[i_index]) / this->existing_signal_input_standard_deviations[i_index];
				sum_vals += this->existing_signal_weights[i_index] * normalized_val;
			}
		}
		history->existing_predicted_scores.push_back(sum_vals);

		this->scope_histories.push_back(new ScopeHistory(wrapper->scope_histories.back()));

		uniform_int_distribution<int> until_distribution(0, (int)this->average_instances_per_run-1.0);
		this->num_instances_until_target = 1 + until_distribution(generator);

		wrapper->scope_histories.back()->experiments_hit.push_back(this);

		BranchCompareExperimentState* new_experiment_state = new BranchCompareExperimentState(this);
		new_experiment_state->step_index = 0;
		wrapper->experiment_context.back() = new_experiment_state;
	}
}

void BranchCompareExperiment::train_new_step(
		vector<double>& obs,
		int& action,
		bool& is_next,
		SolutionWrapper* wrapper,
		BranchCompareExperimentState* experiment_state) {
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
		}
	}
}

void BranchCompareExperiment::train_new_exit_step(
		SolutionWrapper* wrapper,
		BranchCompareExperimentState* experiment_state) {
	this->best_scopes[experiment_state->step_index]->back_activate(wrapper);

	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void BranchCompareExperiment::train_new_back_activate(
		SolutionWrapper* wrapper) {
	BranchCompareExperimentHistory* history = (BranchCompareExperimentHistory*)wrapper->experiment_history;

	double reward_signal = calc_reward_signal(wrapper->scope_histories.back());
	this->signal_target_val_histories.push_back(reward_signal - history->existing_predicted_scores.back());
}

void BranchCompareExperiment::train_new_backprop(
		double target_val,
		BranchCompareExperimentHistory* history) {
	if (history->is_hit) {
		for (int i_index = 0; i_index < (int)history->existing_predicted_scores.size(); i_index++) {
			this->true_target_val_histories.push_back(target_val - history->existing_predicted_scores[i_index]);
		}

		this->state_iter++;
		if (this->state_iter >= TRAIN_NEW_NUM_DATAPOINTS
				&& (int)this->true_target_val_histories.size() >= TRAIN_NEW_NUM_DATAPOINTS) {
			{
				double average_score;
				vector<Input> factor_inputs;
				vector<double> factor_input_averages;
				vector<double> factor_input_standard_deviations;
				vector<double> factor_weights;
				vector<Input> network_inputs;
				Network* network = NULL;
				double select_percentage;
				bool is_success = train_new(this->scope_histories,
											this->true_target_val_histories,
											average_score,
											factor_inputs,
											factor_input_averages,
											factor_input_standard_deviations,
											factor_weights,
											network_inputs,
											network,
											select_percentage);

				if (!is_success) {
					if (network != NULL) {
						delete network;
					}

					this->result = EXPERIMENT_RESULT_FAIL;

					return;
				}

				this->new_true_average_score = average_score;
				this->new_true_inputs = factor_inputs;
				this->new_true_input_averages = factor_input_averages;
				this->new_true_input_standard_deviations = factor_input_standard_deviations;
				this->new_true_weights = factor_weights;
				this->new_true_network_inputs = network_inputs;
				this->new_true_network = network;
			}

			{
				double average_score;
				vector<Input> factor_inputs;
				vector<double> factor_input_averages;
				vector<double> factor_input_standard_deviations;
				vector<double> factor_weights;
				vector<Input> network_inputs;
				Network* network = NULL;
				double select_percentage;
				bool is_success = train_new(this->scope_histories,
											this->signal_target_val_histories,
											average_score,
											factor_inputs,
											factor_input_averages,
											factor_input_standard_deviations,
											factor_weights,
											network_inputs,
											network,
											select_percentage);

				if (!is_success) {
					if (network != NULL) {
						delete network;
					}

					this->result = EXPERIMENT_RESULT_FAIL;

					return;
				}

				this->new_signal_average_score = average_score;
				this->new_signal_inputs = factor_inputs;
				this->new_signal_input_averages = factor_input_averages;
				this->new_signal_input_standard_deviations = factor_input_standard_deviations;
				this->new_signal_weights = factor_weights;
				this->new_signal_network_inputs = network_inputs;
				this->new_signal_network = network;
			}

			for (int h_index = 0; h_index < (int)this->scope_histories.size(); h_index++) {
				delete this->scope_histories[h_index];
			}
			this->scope_histories.clear();
			this->true_target_val_histories.clear();
			this->signal_target_val_histories.clear();

			this->new_branch_node = new BranchNode();
			this->new_branch_node->parent = this->scope_context;
			this->new_branch_node->id = this->scope_context->node_counter;
			this->scope_context->node_counter++;

			ObsNode* start_obs_node = new ObsNode();
			start_obs_node->parent = this->scope_context;
			start_obs_node->id = this->scope_context->node_counter;
			this->scope_context->node_counter++;

			this->new_nodes.push_back(start_obs_node);

			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					ActionNode* new_action_node = new ActionNode();
					new_action_node->parent = this->scope_context;
					new_action_node->id = this->scope_context->node_counter;
					this->scope_context->node_counter++;

					new_action_node->action = this->best_actions[s_index];

					this->new_nodes.push_back(new_action_node);
				} else {
					ScopeNode* new_scope_node = new ScopeNode();
					new_scope_node->parent = this->scope_context;
					new_scope_node->id = this->scope_context->node_counter;
					this->scope_context->node_counter++;

					new_scope_node->scope = this->best_scopes[s_index];

					this->new_nodes.push_back(new_scope_node);
				}

				ObsNode* new_obs_node = new ObsNode();
				new_obs_node->parent = this->scope_context;
				new_obs_node->id = this->scope_context->node_counter;
				this->scope_context->node_counter++;

				this->new_nodes.push_back(new_obs_node);
			}

			this->state = BRANCH_COMPARE_EXPERIMENT_STATE_MEASURE_TRUE;
			this->state_iter = 0;
		}
	}
}
