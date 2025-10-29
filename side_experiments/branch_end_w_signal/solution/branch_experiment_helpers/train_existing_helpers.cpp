#include "branch_experiment.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "network.h"
#include "solution.h"
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

	history->signal_sum_vals.push_back(0.0);
	history->signal_sum_counts.push_back(0);

	wrapper->experiment_callbacks.push_back(wrapper->branch_node_stack);

	delete experiment_state;
	wrapper->experiment_context.back() = NULL;
}

void BranchExperiment::train_existing_backprop(
		double target_val,
		SolutionWrapper* wrapper) {
	BranchExperimentHistory* history = (BranchExperimentHistory*)wrapper->experiment_history;
	if (history->is_hit) {
		for (int s_index = 0; s_index < (int)history->signal_sum_vals.size(); s_index++) {
			history->signal_sum_vals[s_index] += (target_val - wrapper->solution->curr_score);
			history->signal_sum_counts[s_index]++;

			double average_val = history->signal_sum_vals[s_index] / history->signal_sum_counts[s_index];

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
