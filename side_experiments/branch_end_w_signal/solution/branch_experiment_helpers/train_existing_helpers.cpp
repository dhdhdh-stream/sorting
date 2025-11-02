#include "branch_experiment.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "network.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_EXISTING_NUM_DATAPOINTS = 20;
#else
const int TRAIN_EXISTING_NUM_DATAPOINTS = 1000;
#endif /* MDEBUG */

void BranchExperiment::train_existing_check_activate(
		SolutionWrapper* wrapper) {
	this->sum_instances++;

	BranchExperimentState* new_experiment_state = new BranchExperimentState(this);
	new_experiment_state->step_index = 0;
	wrapper->experiment_context.back() = new_experiment_state;
}

void BranchExperiment::train_existing_step(vector<double>& obs,
										   int& action,
										   bool& is_next,
										   SolutionWrapper* wrapper) {
	BranchExperimentHistory* history = (BranchExperimentHistory*)wrapper->experiment_history;
	BranchExperimentState* experiment_state = (BranchExperimentState*)wrapper->experiment_context.back();

	this->obs_histories.push_back(obs);

	history->stack_traces.push_back(wrapper->scope_histories);

	delete experiment_state;
	wrapper->experiment_context.back() = NULL;
}

void BranchExperiment::train_existing_backprop(
		double target_val,
		SolutionWrapper* wrapper) {
	add_existing_samples(wrapper->scope_histories[0],
						 target_val);

	BranchExperimentHistory* history = (BranchExperimentHistory*)wrapper->experiment_history;
	if (history->is_hit) {
		for (int s_index = 0; s_index < (int)history->stack_traces.size(); s_index++) {
			double sum_vals = target_val - wrapper->solution->curr_score;
			int sum_counts = 1;

			for (int l_index = 0; l_index < (int)history->stack_traces[s_index].size(); l_index++) {
				ScopeHistory* scope_history = history->stack_traces[s_index][l_index];
				Scope* scope = scope_history->scope;
				if (scope->consistency_network != NULL) {
					if (!scope_history->signal_initialized) {
						vector<double> inputs = scope_history->pre_obs;
						inputs.insert(inputs.end(), scope_history->post_obs.begin(), scope_history->post_obs.end());

						scope->consistency_network->activate(inputs);
						scope_history->signal_initialized = true;
						#if defined(MDEBUG) && MDEBUG
						scope_history->consistency_val = 2 * (rand()%2) - 1;
						#else
						scope_history->consistency_val = scope->consistency_network->output->acti_vals[0];
						#endif /* MDEBUG */

						scope->pre_network->activate(scope_history->pre_obs);
						scope_history->pre_val = scope->pre_network->output->acti_vals[0];

						scope->post_network->activate(inputs);
						scope_history->post_val = scope->post_network->output->acti_vals[0];
					}

					sum_vals += (scope_history->post_val - scope_history->pre_val);
					sum_counts++;
				}
			}

			double average_val = sum_vals / sum_counts;

			this->target_val_histories.push_back(average_val);
		}

		this->sum_scores += target_val;

		this->state_iter++;
		if (this->state_iter >= TRAIN_EXISTING_NUM_DATAPOINTS) {
			this->existing_score = this->sum_scores / this->state_iter;

			this->existing_network = new Network(this->obs_histories[0].size(),
												 NETWORK_SIZE_SMALL);
			uniform_int_distribution<int> input_distribution(0, this->obs_histories.size()-1);
			for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
				int rand_index = input_distribution(generator);

				this->existing_network->activate(this->obs_histories[rand_index]);

				double error = this->target_val_histories[rand_index] - this->existing_network->output->acti_vals[0];

				this->existing_network->backprop(error);
			}

			this->obs_histories.clear();
			this->target_val_histories.clear();

			this->average_instances_per_run = this->sum_instances / TRAIN_EXISTING_NUM_DATAPOINTS;

			this->best_surprise = numeric_limits<double>::lowest();

			uniform_int_distribution<int> until_distribution(1, 2 * this->average_instances_per_run);
			this->num_instances_until_target = until_distribution(generator);

			this->state = BRANCH_EXPERIMENT_STATE_EXPLORE;
			this->state_iter = 0;
		}
	}
}
