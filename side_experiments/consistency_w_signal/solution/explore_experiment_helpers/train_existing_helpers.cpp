#include "explore_experiment.h"

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

void ExploreExperiment::train_existing_check_activate(SolutionWrapper* wrapper) {
	ExploreExperimentState* new_experiment_state = new ExploreExperimentState(this);
	new_experiment_state->step_index = 0;
	wrapper->experiment_context.back() = new_experiment_state;
}

void ExploreExperiment::train_existing_step(vector<double>& obs,
											SolutionWrapper* wrapper) {
	ExploreExperimentHistory* history = (ExploreExperimentHistory*)wrapper->experiment_history;

	this->obs_histories.push_back(obs);

	history->stack_traces.push_back(wrapper->scope_histories);

	this->sum_num_instances++;

	delete wrapper->experiment_context.back();
	wrapper->experiment_context.back() = NULL;
}

void ExploreExperiment::train_existing_backprop(double target_val,
												SolutionWrapper* wrapper) {
	this->total_count++;

	add_existing_samples_helper(wrapper->scope_histories[0]);

	ExploreExperimentHistory* history = (ExploreExperimentHistory*)wrapper->experiment_history;
	if (history->is_hit) {
		for (int s_index = 0; s_index < (int)history->stack_traces.size(); s_index++) {
			double sum_vals = 0.0;
			int sum_counts = 0;

			for (int l_index = 0; l_index < (int)history->stack_traces[s_index].size(); l_index++) {
				ScopeHistory* scope_history = history->stack_traces[s_index][l_index];
				Scope* scope = scope_history->scope;
				if (scope->pre_network != NULL) {
					if (!scope_history->signal_initialized) {
						scope->pre_network->activate(scope_history->pre_obs);
						scope_history->pre_val = scope->pre_network->output->acti_vals[0];

						vector<double> inputs = scope_history->pre_obs;
						inputs.insert(inputs.end(), scope_history->post_obs.begin(), scope_history->post_obs.end());

						scope->post_network->activate(inputs);
						scope_history->post_val = scope->post_network->output->acti_vals[0];

						scope_history->signal_initialized = true;
					}

					sum_vals += (scope_history->post_val - scope_history->pre_val);
					sum_counts++;
				}
			}

			// // temp
			// {
			// 	double average_signal;
			// 	if (sum_counts == 0) {
			// 		average_signal = 0.0;
			// 	} else {
			// 		average_signal = sum_vals / sum_counts;
			// 	}
			// 	this->signal_histories.push_back(average_signal);
			// }

			sum_vals += (target_val - wrapper->solution->curr_score);
			sum_counts++;

			double average_val = sum_vals / sum_counts;
			this->target_val_histories.push_back(average_val);
		}

		this->sum_scores += target_val;

		this->state_iter++;
		if (this->state_iter >= TRAIN_EXISTING_NUM_DATAPOINTS) {
			this->existing_score = this->sum_scores / this->state_iter;
			// // temp
			// {
			// 	double sum_vals = 0.0;
			// 	for (int h_index = 0; h_index < (int)this->signal_histories.size(); h_index++) {
			// 		sum_vals += this->signal_histories[h_index];
			// 	}
			// 	this->existing_signal = sum_vals / (double)this->signal_histories.size();
			// }

			this->average_hits_per_run = (double)TRAIN_EXISTING_NUM_DATAPOINTS / (double)this->total_count;

			this->existing_network = new Network(this->obs_histories[0].size(),
												 NETWORK_SIZE_SMALL);
			uniform_int_distribution<int> val_input_distribution(0, this->obs_histories.size()-1);
			for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
				int rand_index = val_input_distribution(generator);

				this->existing_network->activate(this->obs_histories[rand_index]);

				double error = this->target_val_histories[rand_index] - this->existing_network->output->acti_vals[0];

				this->existing_network->backprop(error);
			}

			this->average_instances_per_run = this->sum_num_instances / (double)TRAIN_EXISTING_NUM_DATAPOINTS;

			uniform_int_distribution<int> until_distribution(1, 2 * this->average_instances_per_run);
			this->num_instances_until_target = until_distribution(generator);

			this->state = EXPLORE_EXPERIMENT_STATE_EXPLORE;
			this->state_iter = 0;
		}
	}
}
