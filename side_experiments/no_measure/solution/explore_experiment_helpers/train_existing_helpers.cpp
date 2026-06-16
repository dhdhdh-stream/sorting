#include "explore_experiment.h"

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
const int TRAIN_EXISTING_NUM_DATAPOINTS = 4000;
#endif /* MDEBUG */

void ExploreExperiment::train_existing_check_activate(SolutionWrapper* wrapper) {
	ExploreExperimentState* new_experiment_state = new ExploreExperimentState(this);
	new_experiment_state->step_index = 0;
	wrapper->experiment_context.back() = new_experiment_state;
}

void ExploreExperiment::train_existing_step(
		vector<double>& obs,
		SolutionWrapper* wrapper) {
	this->obs_histories.push_back(obs);

	this->sum_num_instances++;

	delete wrapper->experiment_context.back();
	wrapper->experiment_context.back() = NULL;
}

void ExploreExperiment::train_existing_backprop(
		double target_val,
		ExploreExperimentHistory* history,
		SolutionWrapper* wrapper) {
	while (this->target_val_histories.size() < this->obs_histories.size()) {
		this->target_val_histories.push_back(target_val);
	}

	this->state_iter++;
	if (this->state_iter >= TRAIN_EXISTING_NUM_DATAPOINTS) {
		this->average_instances_per_run = (double)this->sum_num_instances / this->state_iter;

		uniform_int_distribution<int> val_input_distribution(0, this->obs_histories.size()-1);

		this->existing_network = new Network(this->obs_histories[0].size(),
											 NETWORK_SIZE_SMALL);
		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
			int rand_index = val_input_distribution(generator);

			this->existing_network->activate(this->obs_histories[rand_index]);

			double error = this->target_val_histories[rand_index] - this->existing_network->output->acti_vals[0];

			this->existing_network->backprop(error);
		}

		this->obs_histories.clear();
		this->target_val_histories.clear();

		this->best_surprise = numeric_limits<double>::lowest();

		uniform_int_distribution<int> until_distribution(1, 2 * this->average_instances_per_run);
		this->num_instances_until_target = until_distribution(generator);

		this->state = EXPLORE_EXPERIMENT_STATE_EXPLORE;
		this->state_iter = 0;
	}
}
