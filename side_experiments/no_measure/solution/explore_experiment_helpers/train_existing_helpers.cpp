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

void ExploreExperiment::train_existing_check_activate(SolutionWrapper* wrapper) {
	ExploreExperimentState* new_experiment_state = new ExploreExperimentState(this);
	new_experiment_state->step_index = 0;
	wrapper->experiment_context.back() = new_experiment_state;
}

void ExploreExperiment::train_existing_step(
		vector<double>& obs,
		SolutionWrapper* wrapper) {
	ExploreExperimentHistory* history = wrapper->explore_experiment_histories[this];
	history->obs_histories.push_back(obs);

	delete wrapper->experiment_context.back();
	wrapper->experiment_context.back() = NULL;
}

void ExploreExperiment::train_existing_backprop(
		double target_val,
		ExploreExperimentHistory* history,
		SolutionWrapper* wrapper) {
	for (int i_index = 0; i_index < (int)history->obs_histories.size(); i_index++) {
		this->existing_obs_histories.push_back(history->obs_histories[i_index]);
		this->existing_target_val_histories.push_back(target_val);
	}

	this->state_iter++;
	if (this->state_iter >= EXPERIMENT_NUM_DATAPOINTS) {
		{
			default_random_engine generator_copy = generator;
			shuffle(this->existing_obs_histories.begin(), this->existing_obs_histories.end(), generator_copy);
		}
		{
			default_random_engine generator_copy = generator;
			shuffle(this->existing_target_val_histories.begin(), this->existing_target_val_histories.end(), generator_copy);
		}

		int num_existing_train = (1.0 - VERIFY_RATIO) * (double)this->existing_obs_histories.size();

		vector<double> sum_means(this->existing_obs_histories[0].size(), 0.0);
		for (int h_index = 0; h_index < num_existing_train; h_index++) {
			for (int i_index = 0; i_index < (int)this->existing_obs_histories[0].size(); i_index++) {
				sum_means[i_index] += this->existing_obs_histories[h_index][i_index];
			}
		}
		vector<double> means(this->existing_obs_histories[0].size());
		for (int i_index = 0; i_index < (int)this->existing_obs_histories[0].size(); i_index++) {
			means[i_index] = sum_means[i_index] / num_existing_train;
		}
		vector<double> sum_variances(this->existing_obs_histories[0].size(), 0.0);
		for (int h_index = 0; h_index < num_existing_train; h_index++) {
			for (int i_index = 0; i_index < (int)this->existing_obs_histories[0].size(); i_index++) {
				sum_variances[i_index] += (this->existing_obs_histories[h_index][i_index] - means[i_index])
					* (this->existing_obs_histories[h_index][i_index] - means[i_index]);
			}
		}
		vector<double> deviations(this->existing_obs_histories[0].size());
		for (int i_index = 0; i_index < (int)this->existing_obs_histories[0].size(); i_index++) {
			deviations[i_index] = sqrt(sum_variances[i_index] / num_existing_train);
		}

		this->existing_network = new Network(this->existing_obs_histories[0].size(),
											 means,
											 deviations);
		double hidden_1_average_max_update = 0.0;
		double hidden_2_average_max_update = 0.0;
		double hidden_3_average_max_update = 0.0;
		double output_average_max_update = 0.0;

		uniform_int_distribution<int> train_distribution(0, num_existing_train-1);
		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
			int rand_index = train_distribution(generator);

			this->existing_network->activate(this->existing_obs_histories[rand_index]);

			double error = this->existing_target_val_histories[rand_index] - this->existing_network->output->acti_vals[0];

			this->existing_network->init_backprop(error,
												  hidden_1_average_max_update,
												  hidden_2_average_max_update,
												  hidden_3_average_max_update,
												  output_average_max_update);
		}

		this->existing_obs_histories.clear();
		this->existing_target_val_histories.clear();

		this->best_surprise = numeric_limits<double>::lowest();

		uniform_int_distribution<int> until_distribution(1, 2 * this->average_instances_per_run);
		this->num_instances_until_target = until_distribution(generator);

		this->state = EXPLORE_EXPERIMENT_STATE_EXPLORE;
		this->state_iter = 0;
	}
}
