#include "experiment.h"

#include <algorithm>
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
const int TRAIN_EXISTING_NUM_DATAPOINTS = 800;
#endif /* MDEBUG */

void Experiment::train_existing_check_activate(SolutionWrapper* wrapper) {
	ExperimentState* new_experiment_state = new ExperimentState(this);
	new_experiment_state->step_index = 0;
	wrapper->experiment_context.back() = new_experiment_state;
}

void Experiment::train_existing_step(vector<double>& obs,
									 SolutionWrapper* wrapper) {
	this->existing_obs_histories.push_back(obs);

	ExperimentHistory* history = (ExperimentHistory*)wrapper->experiment_history;
	history->stack_traces.push_back(wrapper->scope_histories);

	this->sum_num_instances++;

	delete wrapper->experiment_context.back();
	wrapper->experiment_context.back() = NULL;
}

void Experiment::train_existing_backprop(double target_val,
										 SolutionWrapper* wrapper) {
	ExperimentHistory* history = (ExperimentHistory*)wrapper->experiment_history;

	if (history->is_hit) {
		if (wrapper->curr_tunnel == NULL) {
			this->sum_true += target_val;
		} else {
			double sum_vals = 0.0;
			eval_curr_tunnel_helper(wrapper->scope_histories[0],
									wrapper,
									sum_vals);
			this->sum_true += sum_vals;
		}
		this->hit_count++;

		if (wrapper->curr_tunnel == NULL) {
			for (int i_index = 0; i_index < (int)history->stack_traces.size(); i_index++) {
				this->existing_true_histories.push_back(target_val);
			}
		} else {
			for (int i_index = 0; i_index < (int)history->stack_traces.size(); i_index++) {
				this->scope_context->post_network->activate(history->stack_traces[i_index].back()->post_obs_history);
				this->existing_true_histories.push_back(this->scope_context->post_network->output->acti_vals[0]);
			}
		}
	}

	if (this->hit_count >= TRAIN_EXISTING_NUM_DATAPOINTS) {
		this->existing_true = this->sum_true / this->hit_count;

		uniform_int_distribution<int> val_input_distribution(0, this->existing_obs_histories.size()-1);

		this->existing_true_network = new Network(this->existing_obs_histories[0].size(),
												  NETWORK_SIZE_SMALL);
		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
			int rand_index = val_input_distribution(generator);

			this->existing_true_network->activate(this->existing_obs_histories[rand_index]);

			double error = this->existing_true_histories[rand_index] - this->existing_true_network->output->acti_vals[0];

			this->existing_true_network->backprop(error);
		}

		this->average_instances_per_run = (double)this->sum_num_instances / (double)this->hit_count;

		this->best_surprise = numeric_limits<double>::lowest();

		uniform_int_distribution<int> until_distribution(1, 2 * this->average_instances_per_run);
		this->num_instances_until_target = until_distribution(generator);

		this->state = EXPERIMENT_STATE_EXPLORE;
		this->state_iter = 0;
	}
}
