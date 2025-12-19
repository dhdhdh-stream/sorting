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

#if defined(MDEBUG) && MDEBUG
const int TRAIN_SIGNAL_ITERS = 50;
#else
const int TRAIN_SIGNAL_ITERS = 500000;
#endif /* MDEBUG */

void Scope::update_signals(SolutionWrapper* wrapper) {
	if (this->existing_pre_obs.size() >= 3
			&& this->existing_pre_obs.back().size() > 0) {
		SignalExperiment* new_signal_experiment = new SignalExperiment();

		uniform_int_distribution<int> is_existing_distribution(0, 1);
		uniform_int_distribution<int> existing_distribution(0, this->existing_pre_obs.size()-1);
		uniform_int_distribution<int> explore_distribution(0, this->explore_pre_obs.size()-1);

		Network* consistency_network = new Network(this->explore_pre_obs[0].size() + this->explore_post_obs[0].size(),
												   NETWORK_SIZE_LARGE);
		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
			if (is_existing_distribution(generator) == 0) {
				int existing_iter = existing_distribution(generator);

				uniform_int_distribution<int> distribution(0, this->existing_pre_obs[existing_iter].size()-1);
				int index = distribution(generator);

				vector<double> input = this->existing_pre_obs[existing_iter][index];
				input.insert(input.end(), this->existing_post_obs[existing_iter][index].begin(), this->existing_post_obs[existing_iter][index].end());

				consistency_network->activate(input);

				double error;
				if (consistency_network->output->acti_vals[0] >= 1.0) {
					error = 0.0;
				} else {
					error = 1.0 - consistency_network->output->acti_vals[0];
				}

				consistency_network->backprop(error);
			} else {
				int index = explore_distribution(generator);

				vector<double> input = this->explore_pre_obs[index];
				input.insert(input.end(), this->explore_post_obs[index].begin(), this->explore_post_obs[index].end());

				consistency_network->activate(input);

				double error;
				if (consistency_network->output->acti_vals[0] <= 0.0) {
					error = 0.0;
				} else {
					error = 0.0 - consistency_network->output->acti_vals[0];
				}

				consistency_network->backprop(error);
			}
		}

		// temp
		{
			double sum_existing_consistency = 0.0;
			int existing_count = 0;
			for (int i_index = 0; i_index < (int)this->existing_pre_obs.size(); i_index++) {
				for (int h_index = 0; h_index < (int)this->existing_pre_obs[i_index].size(); h_index++) {
					vector<double> input = this->existing_pre_obs[i_index][h_index];
					input.insert(input.end(), this->existing_post_obs[i_index][h_index].begin(), this->existing_post_obs[i_index][h_index].end());

					consistency_network->activate(input);

					double consistency = consistency_network->output->acti_vals[0];
					if (consistency < 0.0) {
						consistency = 0.0;
					} else if (consistency > 1.0) {
						consistency = 1.0;
					}

					sum_existing_consistency += consistency;
					existing_count++;
				}
			}
			double existing_average_consistency = sum_existing_consistency / existing_count;
			cout << "existing_average_consistency: " << existing_average_consistency << endl;

			double sum_explore_consistency = 0.0;
			int explore_count = 0;
			for (int h_index = 0; h_index < (int)this->explore_pre_obs.size(); h_index++) {
				vector<double> input = this->explore_pre_obs[h_index];
				input.insert(input.end(), this->explore_post_obs[h_index].begin(), this->explore_post_obs[h_index].end());

				consistency_network->activate(input);

				double consistency = consistency_network->output->acti_vals[0];
				if (consistency < 0.0) {
					consistency = 0.0;
				} else if (consistency > 1.0) {
					consistency = 1.0;
				}

				sum_explore_consistency += consistency;
				explore_count++;
			}
			double explore_average_consistency = sum_explore_consistency / explore_count;
			cout << "explore_average_consistency: " << explore_average_consistency << endl;
		}

		vector<vector<double>> existing_consistency_vals(this->existing_pre_obs.size());
		double sum_existing_consistency = 0.0;
		double sum_adjusted_existing_target_vals = 0.0;
		for (int i_index = 0; i_index < (int)this->existing_pre_obs.size(); i_index++) {
			existing_consistency_vals[i_index] = vector<double>(this->existing_pre_obs[i_index].size());
			for (int h_index = 0; h_index < (int)this->existing_pre_obs[i_index].size(); h_index++) {
				vector<double> input = this->existing_pre_obs[i_index][h_index];
				input.insert(input.end(), this->existing_post_obs[i_index][h_index].begin(), this->existing_post_obs[i_index][h_index].end());

				consistency_network->activate(input);

				double consistency = consistency_network->output->acti_vals[0];
				if (consistency < 0.0) {
					consistency = 0.0;
				} else if (consistency > 1.0) {
					consistency = 1.0;
				}

				existing_consistency_vals[i_index][h_index] = consistency;

				sum_existing_consistency += consistency;
				sum_adjusted_existing_target_vals += consistency * this->existing_target_vals[i_index][h_index];
			}
		}

		vector<double> explore_consistency_vals(this->explore_pre_obs.size());
		double sum_explore_consistency = 0.0;
		double sum_adjusted_explore_target_vals = 0.0;
		for (int h_index = 0; h_index < (int)this->explore_pre_obs.size(); h_index++) {
			vector<double> input = this->explore_pre_obs[h_index];
			input.insert(input.end(), this->explore_post_obs[h_index].begin(), this->explore_post_obs[h_index].end());

			consistency_network->activate(input);

			double consistency = consistency_network->output->acti_vals[0];
			if (consistency < 0.0) {
				consistency = 0.0;
			} else if (consistency > 1.0) {
				consistency = 1.0;
			}

			explore_consistency_vals[h_index] = consistency;

			sum_explore_consistency += consistency;
			sum_adjusted_explore_target_vals += consistency * this->explore_target_vals[h_index];
		}

		/**
		 * - re-average so not biased towards more/less signals
		 */
		double average_val = 0.5 * sum_adjusted_existing_target_vals / sum_existing_consistency
			+ 0.5 * sum_adjusted_explore_target_vals / sum_explore_consistency;

		Network* signal_network = new Network(this->explore_pre_obs[0].size() + this->explore_post_obs[0].size(),
											  NETWORK_SIZE_LARGE);
		for (int iter_index = 0; iter_index < TRAIN_SIGNAL_ITERS; iter_index++) {
			if (is_existing_distribution(generator) == 0) {
				int existing_iter = existing_distribution(generator);

				uniform_int_distribution<int> distribution(0, this->existing_pre_obs[existing_iter].size()-1);
				int index = distribution(generator);

				double consistency = existing_consistency_vals[existing_iter][index];
				if (consistency > 0.0) {
					vector<double> input = this->existing_pre_obs[existing_iter][index];
					input.insert(input.end(), this->existing_post_obs[existing_iter][index].begin(), this->existing_post_obs[existing_iter][index].end());

					signal_network->activate(input);

					double error = (this->existing_target_vals[existing_iter][index] - average_val) - signal_network->output->acti_vals[0];
					error *= consistency;

					signal_network->backprop(error);
				}
			} else {
				int index = explore_distribution(generator);

				double consistency = explore_consistency_vals[index];
				if (consistency > 0.0) {
					vector<double> input = this->explore_pre_obs[index];
					input.insert(input.end(), this->explore_post_obs[index].begin(), this->explore_post_obs[index].end());

					signal_network->activate(input);

					double error = (this->explore_target_vals[index] - average_val) - signal_network->output->acti_vals[0];
					error *= consistency;

					signal_network->backprop(error);
				}
			}
		}

		new_signal_experiment->consistency_network = consistency_network;
		new_signal_experiment->signal_network = signal_network;
		this->signal_experiment = new_signal_experiment;
	}

	// SignalExperiment* new_signal_experiment = new SignalExperiment();

	// uniform_int_distribution<int> explore_distribution(0, this->explore_pre_obs.size()-1);

	// /**
	//  * - re-average so not biased towards more/less signals
	//  */
	// double sum_vals = 0.0;
	// for (int h_index = 0; h_index < (int)this->explore_pre_obs.size(); h_index++) {
	// 	sum_vals += this->explore_target_vals[h_index];
	// }
	// double average_val = sum_vals / (double)this->explore_pre_obs.size();

	// Network* signal_network = new Network(this->explore_pre_obs[0].size() + this->explore_post_obs[0].size(),
	// 									  NETWORK_SIZE_LARGE);
	// for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
	// 	int index = explore_distribution(generator);

	// 	vector<double> input = this->explore_pre_obs[index];
	// 	input.insert(input.end(), this->explore_post_obs[index].begin(), this->explore_post_obs[index].end());

	// 	signal_network->activate(input);

	// 	double error = (this->explore_target_vals[index] - average_val) - signal_network->output->acti_vals[0];

	// 	signal_network->backprop(error);
	// }

	// new_signal_experiment->signal_network = signal_network;
	// this->signal_experiment = new_signal_experiment;
}
