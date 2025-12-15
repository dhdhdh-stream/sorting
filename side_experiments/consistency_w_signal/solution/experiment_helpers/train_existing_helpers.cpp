#include "experiment.h"

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

void Experiment::train_existing_check_activate(SolutionWrapper* wrapper) {
	ExperimentHistory* history = (ExperimentHistory*)wrapper->experiment_history;

	wrapper->has_explore = true;
	history->post_scope_histories.push_back(vector<ScopeHistory*>());

	ExperimentState* new_experiment_state = new ExperimentState(this);
	new_experiment_state->step_index = 0;
	wrapper->experiment_context.back() = new_experiment_state;
}

void Experiment::train_existing_step(vector<double>& obs,
									 SolutionWrapper* wrapper) {
	this->obs_histories.push_back(obs);

	this->sum_num_instances++;

	delete wrapper->experiment_context.back();
	wrapper->experiment_context.back() = NULL;
}

void Experiment::train_existing_backprop(double target_val,
										 SolutionWrapper* wrapper) {
	this->total_count++;

	add_existing_samples_helper(wrapper->scope_histories[0]);

	ExperimentHistory* history = (ExperimentHistory*)wrapper->experiment_history;
	if (history->is_hit) {
		wrapper->has_explore = false;

		for (int i_index = 0; i_index < (int)history->post_scope_histories.size(); i_index++) {
			double signal = calc_signal(history->post_scope_histories[i_index],
										target_val,
										wrapper);
			this->target_val_histories.push_back(signal);
		}

		this->sum_scores += target_val;

		this->state_iter++;
		if (this->state_iter >= TRAIN_EXISTING_NUM_DATAPOINTS) {
			this->existing_score = this->sum_scores / this->state_iter;

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

			this->obs_histories.clear();
			this->target_val_histories.clear();

			this->average_instances_per_run = this->sum_num_instances / (double)TRAIN_EXISTING_NUM_DATAPOINTS;

			this->best_surprise = numeric_limits<double>::lowest();

			uniform_int_distribution<int> until_distribution(1, 2 * this->average_instances_per_run);
			this->num_instances_until_target = until_distribution(generator);

			this->state = EXPERIMENT_STATE_EXPLORE;
			this->state_iter = 0;
		}
	}
}
