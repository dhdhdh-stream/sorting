#include "force_experiment.h"

#include <algorithm>
#include <iostream>

#include "constants.h"
#include "experiment_run.h"
#include "globals.h"
#include "network.h"
#include "solution_helpers.h"

using namespace std;

void ForceExperiment::train_existing_experiment_activate(ExperimentRun* run) {
	ForceExperimentHistory* history = run->force_experiment_histories[this];
	history->branch_obs = run->obs_histories;
	history->branch_actions = run->action_histories;
}

void ForceExperiment::train_existing_backprop(double target_val,
											  ExperimentRun* run,
											  ForceExperimentHistory* history,
											  Wrapper* wrapper) {
	this->existing_branch_obs.push_back(history->branch_obs);
	this->existing_branch_actions.push_back(history->branch_actions);
	this->existing_full_obs.push_back(run->obs_histories);
	this->existing_full_actions.push_back(run->action_histories);
	this->existing_target_vals.push_back(target_val);

	this->state_iter++;
	if (this->state_iter >= EXPERIMENT_NUM_SAMPLES) {
		{
			default_random_engine generator_copy = generator;
			shuffle(this->existing_branch_obs.begin(), this->existing_branch_obs.end(), generator_copy);
		}
		{
			default_random_engine generator_copy = generator;
			shuffle(this->existing_branch_actions.begin(), this->existing_branch_actions.end(), generator_copy);
		}
		{
			default_random_engine generator_copy = generator;
			shuffle(this->existing_full_obs.begin(), this->existing_full_obs.end(), generator_copy);
		}
		{
			default_random_engine generator_copy = generator;
			shuffle(this->existing_full_actions.begin(), this->existing_full_actions.end(), generator_copy);
		}
		{
			default_random_engine generator_copy = generator;
			shuffle(this->existing_target_vals.begin(), this->existing_target_vals.end(), generator_copy);
		}

		int num_verify = VERIFICATION_RATIO * (double)this->existing_branch_obs.size();
		int num_train = (int)this->existing_branch_obs.size() - num_verify;

		uniform_int_distribution<int> sample_distribution(0, num_train-1);

		vector<vector<double>> train_existing_states(num_train);
		for (int h_index = 0; h_index < num_train; h_index++) {
			vector<double> state;
			calc_state_helper(this->new_branch_obs[h_index],
							  this->new_branch_actions[h_index],
							  wrapper,
							  state);
			train_existing_states[h_index] = state;
		}

		this->original_network = new Network(train_existing_states[0].size());
		double existing_hidden_1_average_max_update = 0.0;
		double existing_hidden_2_average_max_update = 0.0;
		double existing_hidden_3_average_max_update = 0.0;
		double existing_output_average_max_update = 0.0;
		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
			int index = sample_distribution(generator);
			this->original_network->activate(train_existing_states[index]);
			double error = this->existing_target_vals[index] - this->original_network->output->acti_vals[0];
			this->original_network->init_backprop(error,
												  existing_hidden_1_average_max_update,
												  existing_hidden_2_average_max_update,
												  existing_hidden_3_average_max_update,
												  existing_output_average_max_update);
		}

		this->best_surprise = 0.0;

		this->state = FORCE_EXPERIMENT_STATE_EXPLORE;
		this->state_iter = 0;
	}
}
