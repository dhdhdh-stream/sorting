#include "chase_experiment.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "network.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "tunnel.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_EXISTING_NUM_DATAPOINTS = 20;
#else
const int TRAIN_EXISTING_NUM_DATAPOINTS = 1000;
#endif /* MDEBUG */

void ChaseExperiment::train_existing_check_activate(SolutionWrapper* wrapper) {
	ChaseExperimentState* new_experiment_state = new ChaseExperimentState(this);
	new_experiment_state->step_index = 0;
	wrapper->experiment_context.back() = new_experiment_state;
}

void ChaseExperiment::train_existing_step(vector<double>& obs,
										  SolutionWrapper* wrapper) {
	this->obs_histories.push_back(obs);

	this->sum_num_instances++;

	delete wrapper->experiment_context.back();
	wrapper->experiment_context.back() = NULL;
}

void ChaseExperiment::train_existing_backprop(double target_val,
											  SolutionWrapper* wrapper) {
	this->total_count++;

	ChaseExperimentHistory* history = (ChaseExperimentHistory*)wrapper->experiment_history;
	if (history->is_hit) {
		while (this->true_histories.size() < this->obs_histories.size()) {
			this->true_histories.push_back(target_val);
		}

		double signal;
		if (wrapper->tunnel == NULL) {
			signal = target_val;
		} else {
			signal = wrapper->tunnel->get_signal(wrapper);
		}
		while (this->signal_histories.size() < this->obs_histories.size()) {
			this->signal_histories.push_back(signal);
		}

		this->sum_true += target_val;
		this->sum_signal += signal;

		this->state_iter++;
		if (this->state_iter >= TRAIN_EXISTING_NUM_DATAPOINTS) {
			this->existing_true = this->sum_true / this->state_iter;
			this->existing_signal = this->sum_signal / this->state_iter;

			this->average_hits_per_run = (double)TRAIN_EXISTING_NUM_DATAPOINTS / (double)this->total_count;

			uniform_int_distribution<int> val_input_distribution(0, this->obs_histories.size()-1);

			this->existing_true_network = new Network(this->obs_histories[0].size(),
													  NETWORK_SIZE_SMALL);
			for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
				int rand_index = val_input_distribution(generator);

				this->existing_true_network->activate(this->obs_histories[rand_index]);

				double error = this->true_histories[rand_index] - this->existing_true_network->output->acti_vals[0];

				this->existing_true_network->backprop(error);
			}

			this->existing_signal_network = new Network(this->obs_histories[0].size(),
														NETWORK_SIZE_SMALL);
			for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
				int rand_index = val_input_distribution(generator);

				this->existing_signal_network->activate(this->obs_histories[rand_index]);

				double error = this->signal_histories[rand_index] - this->existing_signal_network->output->acti_vals[0];

				this->existing_signal_network->backprop(error);
			}

			this->obs_histories.clear();
			this->true_histories.clear();
			this->signal_histories.clear();

			this->average_instances_per_run = this->sum_num_instances / (double)TRAIN_EXISTING_NUM_DATAPOINTS;

			this->best_surprise = numeric_limits<double>::lowest();

			uniform_int_distribution<int> until_distribution(1, 2 * this->average_instances_per_run);
			this->num_instances_until_target = until_distribution(generator);

			this->state = CHASE_EXPERIMENT_STATE_EXPLORE;
			this->state_iter = 0;
		}
	}
}
