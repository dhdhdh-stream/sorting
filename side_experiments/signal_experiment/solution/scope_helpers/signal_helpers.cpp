#include "scope.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "network.h"
#include "signal_experiment.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

void Scope::update_signals(SolutionWrapper* wrapper) {
	if (this->explore_pre_obs.size() >= MIN_TRAIN_SAMPLES) {
		SignalExperiment* new_signal_experiment = new SignalExperiment();

		/**
		 * - re-average so not biased towards more/less signals
		 */
		double sum_explore_vals = 0.0;
		for (int h_index = 0; h_index < (int)this->explore_pre_obs.size(); h_index++) {
			sum_explore_vals += this->explore_target_vals[h_index];
		}
		double explore_val_average = sum_explore_vals / (double)this->explore_pre_obs.size();
		for (int h_index = 0; h_index < (int)this->explore_pre_obs.size(); h_index++) {
			this->explore_target_vals[h_index] -= explore_val_average;
		}

		Network* signal_network = new Network(this->explore_pre_obs[0].size() + this->explore_post_obs[0].size(),
											  NETWORK_SIZE_LARGE);

		uniform_int_distribution<int> distribution(0, this->explore_pre_obs.size()-1);
		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
			int index = distribution(generator);

			vector<double> input = this->explore_pre_obs[index];
			input.insert(input.end(), this->explore_post_obs[index].begin(), this->explore_post_obs[index].end());

			signal_network->activate(input);

			double post_error = this->explore_target_vals[index] - signal_network->output->acti_vals[0];

			signal_network->backprop(post_error);
		}

		new_signal_experiment->network = signal_network;
		this->signal_experiment = new_signal_experiment;
	}
}
