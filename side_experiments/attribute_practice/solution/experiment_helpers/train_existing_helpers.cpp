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

	if (this->signal_depth != -1) {
		ExperimentHistory* history = (ExperimentHistory*)wrapper->experiment_history;

		history->starting_impact.push_back(wrapper->curr_impact);
		history->ending_impact.push_back(0.0);
		if (this->signal_depth >= (int)wrapper->scope_histories.size()) {
			wrapper->scope_histories[0]->experiment_callback_histories.push_back(history);
			wrapper->scope_histories[0]->experiment_callback_indexes.push_back(history->ending_impact.size()-1);
		} else {
			int index = wrapper->scope_histories.size()-1 - this->signal_depth;
			wrapper->scope_histories[index]->experiment_callback_histories.push_back(history);
			wrapper->scope_histories[index]->experiment_callback_indexes.push_back(history->ending_impact.size()-1);
		}
	}

	this->sum_num_instances++;

	delete wrapper->experiment_context.back();
	wrapper->experiment_context.back() = NULL;
}

void Experiment::train_existing_backprop(double target_val,
										 SolutionWrapper* wrapper) {
	ExperimentHistory* history = (ExperimentHistory*)wrapper->experiment_history;

	if (history->is_hit) {
		this->sum_true += target_val;
		this->hit_count++;

		if (this->signal_depth == -1) {
			while (this->existing_target_vals.size() < this->existing_obs_histories.size()) {
				this->existing_target_vals.push_back(target_val);
			}
		} else {
			for (int i_index = 0; i_index < (int)history->starting_impact.size(); i_index++) {
				this->existing_target_vals.push_back(
					history->ending_impact[i_index] - history->starting_impact[i_index]);
			}
		}
	}

	if (this->hit_count >= TRAIN_EXISTING_NUM_DATAPOINTS) {
		this->existing_true = this->sum_true / this->hit_count;

		uniform_int_distribution<int> val_input_distribution(0, this->existing_obs_histories.size()-1);

		this->existing_network = new Network(this->existing_obs_histories[0].size(),
											 NETWORK_SIZE_SMALL);
		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
			int rand_index = val_input_distribution(generator);

			this->existing_network->activate(this->existing_obs_histories[rand_index]);

			double error = this->existing_target_vals[rand_index] - this->existing_network->output->acti_vals[0];

			this->existing_network->backprop(error);
		}

		this->average_instances_per_run = (double)this->sum_num_instances / (double)this->hit_count;

		this->best_surprise = numeric_limits<double>::lowest();

		uniform_int_distribution<int> until_distribution(1, 2 * this->average_instances_per_run);
		this->num_instances_until_target = until_distribution(generator);

		this->state = EXPERIMENT_STATE_EXPLORE;
		this->state_iter = 0;
	}
}
