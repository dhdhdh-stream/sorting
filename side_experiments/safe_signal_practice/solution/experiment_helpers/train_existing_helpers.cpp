#include "experiment.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "helpers.h"
#include "network.h"
#include "scope.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_EXISTING_NUM_DATAPOINTS = 20;
#else
const int TRAIN_EXISTING_NUM_DATAPOINTS = 4000;
#endif /* MDEBUG */

void Experiment::train_existing_check_activate(
		vector<double>& obs,
		SolutionWrapper* wrapper,
		ExperimentHistory* history) {
	this->obs_histories.push_back(obs);

	this->target_val_histories.push_back(wrapper->prev_clean_result);

	this->signal_histories.push_back(wrapper->prev_signal);
}

void Experiment::train_existing_backprop(double target_val,
										 ExperimentHistory* history,
										 SolutionWrapper* wrapper) {
	this->state_iter++;
	if (this->state_iter >= TRAIN_EXISTING_NUM_DATAPOINTS) {
		// temp
		{
			Scope* scope = wrapper->solution->scopes[0];

			double sum_existing_vals = 0.0;
			for (int h_index = 0; h_index < (int)scope->existing_target_val_histories.size(); h_index++) {
				sum_existing_vals += scope->existing_target_val_histories[h_index];
			}
			double existing_val_average = sum_existing_vals / (double)scope->existing_target_val_histories.size();
			cout << "existing_val_average: " << existing_val_average << endl;

			double sum_explore_vals = 0.0;
			for (int h_index = 0; h_index < (int)scope->explore_target_val_histories.size(); h_index++) {
				sum_explore_vals += scope->explore_target_val_histories[h_index];
			}
			double explore_val_average = sum_explore_vals / (double)scope->explore_target_val_histories.size();
			cout << "explore_val_average: " << explore_val_average << endl;

			double sum_target_vals = 0.0;
			for (int h_index = 0; h_index < (int)this->target_val_histories.size(); h_index++) {
				sum_target_vals += this->target_val_histories[h_index];
			}
			double target_val_average = sum_target_vals / (double)this->target_val_histories.size();
			cout << "target_val_average: " << target_val_average << endl;

			double sum_signals = 0.0;
			for (int h_index = 0; h_index < (int)this->signal_histories.size(); h_index++) {
				sum_signals += this->signal_histories[h_index];
			}
			double signal_average = sum_signals / (double)this->signal_histories.size();
			cout << "signal_average: " << signal_average << endl;
		}

		{
			this->existing_network = new Network(this->obs_histories[0].size());
			uniform_int_distribution<int> distribution(0, this->obs_histories.size()-1);
			for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
				int rand_index = distribution(generator);

				this->existing_network->activate(this->obs_histories[rand_index]);

				double error = this->target_val_histories[rand_index] - this->existing_network->output->acti_vals[0];

				this->existing_network->backprop(error);
			}
		}

		{
			this->existing_signal_network = new Network(this->obs_histories[0].size());
			uniform_int_distribution<int> distribution(0, this->obs_histories.size()-1);
			for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
				int rand_index = distribution(generator);

				this->existing_signal_network->activate(this->obs_histories[rand_index]);

				double error = this->signal_histories[rand_index] - this->existing_signal_network->output->acti_vals[0];

				this->existing_signal_network->backprop(error);
			}
		}

		this->obs_histories.clear();
		this->target_val_histories.clear();

		this->signal_histories.clear();

		this->curr_surprise = 0.0;
		this->curr_new_scope = NULL;
		this->best_surprise = 0.0;
		this->best_new_scope = NULL;

		// temp
		this->num_explore_true_better = 0;
		this->num_explore_signal_better = 0;
		this->num_explore_true_match = 0;
		this->num_explore_signal_match = 0;

		this->state = EXPERIMENT_STATE_EXPLORE;
		this->state_iter = 0;
	}
}
