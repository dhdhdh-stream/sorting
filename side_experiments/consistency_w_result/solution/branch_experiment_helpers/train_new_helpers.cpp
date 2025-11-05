#include "branch_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_NEW_NUM_DATAPOINTS = 20;
#else
const int TRAIN_NEW_NUM_DATAPOINTS = 1000;
#endif /* MDEBUG */

const double MIN_CONSISTENCY_RATIO = 0.2;

void BranchExperiment::train_new_result_check_activate(
		SolutionWrapper* wrapper) {
	BranchExperimentHistory* history = (BranchExperimentHistory*)wrapper->experiment_history;

	uniform_int_distribution<int> select_distribution(0, history->num_instances);
	if (select_distribution(generator) == 0) {
		history->explore_index = history->num_instances;
		history->stack_trace = wrapper->scope_histories;
	}
	history->num_instances++;
}

void BranchExperiment::train_new_result_backprop(SolutionWrapper* wrapper) {
	BranchExperimentHistory* history = (BranchExperimentHistory*)wrapper->experiment_history;

	for (int l_index = 0; l_index < (int)history->stack_trace.size(); l_index++) {
		ScopeHistory* scope_history = history->stack_trace[l_index];
		Scope* scope = scope_history->scope;
		if (scope->consistency_network == NULL) {
			history->existing_predicted_consistency.push_back(0.0);
		} else {
			vector<double> input = scope_history->pre_obs;
			input.insert(input.end(), scope_history->post_obs.begin(),
				scope_history->post_obs.end());

			scope->consistency_network->activate(input);
			history->existing_predicted_consistency.push_back(
				scope->consistency_network->output->acti_vals[0]);
		}
	}

	history->num_instances = 0;
}

void BranchExperiment::train_new_check_activate(
		SolutionWrapper* wrapper) {
	BranchExperimentHistory* history = (BranchExperimentHistory*)wrapper->experiment_history;

	if (history->num_instances == history->explore_index) {
		BranchExperimentState* new_experiment_state = new BranchExperimentState(this);
		new_experiment_state->step_index = 0;
		wrapper->experiment_context.back() = new_experiment_state;
	}

	history->num_instances++;
}

void BranchExperiment::train_new_step(vector<double>& obs,
									  int& action,
									  bool& is_next,
									  SolutionWrapper* wrapper) {
	BranchExperimentState* experiment_state = (BranchExperimentState*)wrapper->experiment_context.back();

	if (experiment_state->step_index == 0) {
		BranchExperimentHistory* history = (BranchExperimentHistory*)wrapper->experiment_history;

		this->obs_histories.push_back(obs);

		history->stack_trace = wrapper->scope_histories;
	}

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

	bool is_consistent = true;
	for (int l_index = 0; l_index < (int)history->stack_trace.size(); l_index++) {
		ScopeHistory* scope_history = history->stack_trace[l_index];
		Scope* scope = scope_history->scope;
		if (scope->consistency_network != NULL) {
			vector<double> inputs = scope_history->pre_obs;
			inputs.insert(inputs.end(), scope_history->post_obs.begin(), scope_history->post_obs.end());

			scope->consistency_network->activate(inputs);

			if (scope->consistency_network->output->acti_vals[0] < history->existing_predicted_consistency[l_index]) {
				is_consistent = false;
			}
		}

		if (scope->signal_status != SIGNAL_STATUS_FAIL) {
			int max_sample_per_timestamp = (TOTAL_MAX_SAMPLES + (int)scope->existing_pre_obs.size() - 1) / (int)scope->existing_pre_obs.size();
			if ((int)scope->explore_pre_obs.back().size() < max_sample_per_timestamp) {
				scope->explore_pre_obs.back().push_back(scope_history->pre_obs);
				scope->explore_post_obs.back().push_back(scope_history->post_obs);
			} else {
				uniform_int_distribution<int> distribution(0, scope->explore_pre_obs.back().size()-1);
				int index = distribution(generator);
				scope->explore_pre_obs.back()[index] = scope_history->pre_obs;
				scope->explore_post_obs.back()[index] = scope_history->post_obs;
			}
		}
	}

	this->target_val_histories.push_back(target_val - wrapper->existing_result);
	this->consistency_histories.push_back(is_consistent);

	this->state_iter++;
	if (this->state_iter >= TRAIN_NEW_NUM_DATAPOINTS
			&& (int)this->target_val_histories.size() >= TRAIN_NEW_NUM_DATAPOINTS) {
		int num_consistent = 0;
		for (int h_index = 0; h_index < (int)this->consistency_histories.size(); h_index++) {
			if (this->consistency_histories[h_index]) {
				num_consistent++;
			}
		}

		if (num_consistent < MIN_CONSISTENCY_RATIO * (double)this->consistency_histories.size()) {
			this->result = EXPERIMENT_RESULT_FAIL;
			return;
		}

		if (num_consistent != (int)this->consistency_histories.size()) {
			this->new_consistency_network = new Network(this->obs_histories[0].size(),
														NETWORK_SIZE_SMALL);
			uniform_int_distribution<int> consistency_input_distribution(0, this->obs_histories.size()-1);
			for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
				int rand_index = consistency_input_distribution(generator);

				this->new_consistency_network->activate(this->obs_histories[rand_index]);

				double error;
				if (this->consistency_histories[rand_index]) {
					if (this->new_consistency_network->output->acti_vals[0] >= 1.0) {
						error = 0.0;
					} else {
						error = 1.0 - this->new_consistency_network->output->acti_vals[0];
					}
				} else {
					if (this->new_consistency_network->output->acti_vals[0] <= -1.0) {
						error = 0.0;
					} else {
						error = -1.0 - this->new_consistency_network->output->acti_vals[0];
					}
				}

				this->new_consistency_network->backprop(error);
			}
		}

		this->new_val_network = new Network(this->obs_histories[0].size(),
											NETWORK_SIZE_SMALL);
		vector<vector<double>> consistent_obs_histories;
		vector<double> consistent_target_val_histories;
		for (int h_index = 0; h_index < (int)this->obs_histories.size(); h_index++) {
			if (this->consistency_histories[h_index]) {
				consistent_obs_histories.push_back(this->obs_histories[h_index]);
				consistent_target_val_histories.push_back(this->target_val_histories[h_index]);
			}
		}
		uniform_int_distribution<int> val_input_distribution(0, consistent_obs_histories.size()-1);
		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
			int rand_index = val_input_distribution(generator);

			this->new_val_network->activate(consistent_obs_histories[rand_index]);

			double error = consistent_target_val_histories[rand_index] - this->new_val_network->output->acti_vals[0];

			this->new_val_network->backprop(error);
		}

		vector<double> network_outputs(consistent_obs_histories.size());
		for (int h_index = 0; h_index < (int)consistent_obs_histories.size(); h_index++) {
			this->new_val_network->activate(consistent_obs_histories[h_index]);

			network_outputs[h_index] = this->new_val_network->output->acti_vals[0];
		}

		int num_positive = 0;
		for (int i_index = 0; i_index < (int)consistent_obs_histories.size(); i_index++) {
			if (network_outputs[i_index] >= 0.0) {
				num_positive++;
			}
		}

		#if defined(MDEBUG) && MDEBUG
		if (num_positive > 0 || rand()%4 != 0) {
		#else
		if (num_positive > 0) {
		#endif /* MDEBUG */
			this->total_count = 0;
			this->total_sum_scores = 0.0;

			this->sum_scores = 0.0;

			this->state = BRANCH_EXPERIMENT_STATE_MEASURE;
			this->state_iter = 0;
		} else {
			this->result = EXPERIMENT_RESULT_FAIL;
		}
	}
}
