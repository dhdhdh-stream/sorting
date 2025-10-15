#include "branch_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_NEW_NUM_DATAPOINTS = 20;
#else
const int TRAIN_NEW_NUM_DATAPOINTS = 200;
#endif /* MDEBUG */

void BranchExperiment::train_new_check_activate(
		SolutionWrapper* wrapper,
		BranchExperimentHistory* history) {
	this->num_instances_until_target--;
	if (this->num_instances_until_target <= 0) {
		double sum_vals = this->existing_constant;
		for (int i_index = 0; i_index < (int)this->existing_inputs.size(); i_index++) {
			double val;
			bool is_on;
			fetch_input_helper(wrapper->scope_histories.back(),
							   this->existing_inputs[i_index],
							   0,
							   val,
							   is_on);
			if (is_on) {
				double normalized_val = (val - this->existing_input_averages[i_index]) / this->existing_input_standard_deviations[i_index];
				sum_vals += this->existing_weights[i_index] * normalized_val;
			}
		}
		if (this->existing_network != NULL) {
			vector<double> input_vals(this->existing_network_inputs.size());
			vector<bool> input_is_on(this->existing_network_inputs.size());
			for (int i_index = 0; i_index < (int)this->existing_network_inputs.size(); i_index++) {
				double val;
				bool is_on;
				fetch_input_helper(wrapper->scope_histories.back(),
								   this->existing_network_inputs[i_index],
								   0,
								   val,
								   is_on);
				input_vals[i_index] = val;
				input_is_on[i_index] = is_on;
			}
			this->existing_network->activate(input_vals,
										input_is_on);
			sum_vals += this->existing_network->output->acti_vals[0];
		}
		history->existing_predicted_scores.push_back(sum_vals);

		ScopeHistory* scope_history_copy = new ScopeHistory(wrapper->scope_histories.back());
		scope_history_copy->num_actions_snapshot = wrapper->num_actions;
		this->scope_histories.push_back(scope_history_copy);

		uniform_int_distribution<int> until_distribution(1, this->average_instances_per_run);
		this->num_instances_until_target = until_distribution(generator);

		BranchExperimentState* new_experiment_state = new BranchExperimentState(this);
		new_experiment_state->step_index = 0;
		wrapper->experiment_context.back() = new_experiment_state;
	}
}

void BranchExperiment::train_new_step(vector<double>& obs,
									  int& action,
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
		}
	}
}

void BranchExperiment::train_new_exit_step(SolutionWrapper* wrapper,
										   BranchExperimentState* experiment_state) {
	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void BranchExperiment::train_new_backprop(
		double target_val,
		SolutionWrapper* wrapper) {
	BranchExperimentHistory* history = (BranchExperimentHistory*)wrapper->experiment_history;
	if (history->is_hit) {
		for (int i_index = 0; i_index < (int)history->existing_predicted_scores.size(); i_index++) {
			this->target_val_histories.push_back((target_val - wrapper->solution->curr_score)
				- history->existing_predicted_scores[i_index]);
		}

		this->state_iter++;
		if (this->state_iter >= TRAIN_NEW_NUM_DATAPOINTS
				&& (int)this->target_val_histories.size() >= TRAIN_NEW_NUM_DATAPOINTS) {
			bool is_success = train_new_helper();
			if (is_success) {
				for (int h_index = 0; h_index < (int)this->scope_histories.size(); h_index++) {
					delete this->scope_histories[h_index];
				}
				this->scope_histories.clear();
				/**
				 * - don't clear this->target_val_histories
				 */

				this->num_refines = 0;

				this->state = BRANCH_EXPERIMENT_STATE_REFINE;
				this->state_iter = 0;
			} else {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}
	}
}
