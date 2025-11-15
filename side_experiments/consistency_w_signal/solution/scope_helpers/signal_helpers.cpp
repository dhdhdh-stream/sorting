#include "scope.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "network.h"

using namespace std;

void Scope::update_consistency() {
	if (this->consistency_explore_pre_obs.size() > MIN_TRAIN_SAMPLES) {
		cout << "this->id: " << this->id << endl;

		if (this->consistency_network == NULL) {
			this->consistency_network = new Network(this->consistency_existing_pre_obs[0].size() + this->consistency_existing_post_obs[0].size(),
													NETWORK_SIZE_LARGE);
			uniform_int_distribution<int> is_existing_distribution(0, 1);
			uniform_int_distribution<int> existing_distribution(0, this->consistency_existing_pre_obs.size()-1);
			uniform_int_distribution<int> explore_distribution(0, this->consistency_explore_pre_obs.size()-1);
			for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
				if (is_existing_distribution(generator) == 0) {
					int index = existing_distribution(generator);

					vector<double> input = this->consistency_existing_pre_obs[index];
					input.insert(input.end(), this->consistency_existing_post_obs[index].begin(), this->consistency_existing_post_obs[index].end());

					this->consistency_network->activate(input);

					double error;
					if (this->consistency_network->output->acti_vals[0] >= 1.0) {
						error = 0.0;
					} else {
						error = 1.0 - this->consistency_network->output->acti_vals[0];
					}

					this->consistency_network->backprop(error);
				} else {
					int index = explore_distribution(generator);

					vector<double> input = this->consistency_explore_pre_obs[index];
					input.insert(input.end(), this->consistency_explore_post_obs[index].begin(), this->consistency_explore_post_obs[index].end());

					this->consistency_network->activate(input);

					double error;
					if (this->consistency_network->output->acti_vals[0] <= -1.0) {
						error = 0.0;
					} else {
						error = -1.0 - this->consistency_network->output->acti_vals[0];
					}

					this->consistency_network->backprop(error);
				}
			}
		} else {
			uniform_int_distribution<int> is_existing_distribution(0, 1);
			uniform_int_distribution<int> existing_distribution(0, this->consistency_existing_pre_obs.size()-1);
			uniform_int_distribution<int> explore_distribution(0, this->consistency_explore_pre_obs.size()-1);
			for (int iter_index = 0; iter_index < UPDATE_ITERS; iter_index++) {
				if (is_existing_distribution(generator) == 0) {
					int index = existing_distribution(generator);

					vector<double> input = this->consistency_existing_pre_obs[index];
					input.insert(input.end(), this->consistency_existing_post_obs[index].begin(), this->consistency_existing_post_obs[index].end());

					this->consistency_network->activate(input);

					double error;
					if (this->consistency_network->output->acti_vals[0] >= 1.0) {
						error = 0.0;
					} else {
						error = 1.0 - this->consistency_network->output->acti_vals[0];
					}

					this->consistency_network->backprop(error);
				} else {
					int index = explore_distribution(generator);

					vector<double> input = this->consistency_explore_pre_obs[index];
					input.insert(input.end(), this->consistency_explore_post_obs[index].begin(), this->consistency_explore_post_obs[index].end());

					this->consistency_network->activate(input);

					double error;
					if (this->consistency_network->output->acti_vals[0] <= -1.0) {
						error = 0.0;
					} else {
						error = -1.0 - this->consistency_network->output->acti_vals[0];
					}

					this->consistency_network->backprop(error);
				}
			}
		}

		double existing_sum_val = 0.0;
		int existing_count = 0;
		double explore_sum_val = 0.0;
		int explore_count = 0;
		for (int h_index = 0; h_index < (int)this->consistency_existing_pre_obs.size(); h_index++) {
			vector<double> input = this->consistency_existing_pre_obs[h_index];
			input.insert(input.end(), this->consistency_existing_post_obs[h_index].begin(), this->consistency_existing_post_obs[h_index].end());

			this->consistency_network->activate(input);

			if (this->consistency_network->output->acti_vals[0] >= 3.0) {
				existing_sum_val += 3.0;
			} else {
				existing_sum_val += this->consistency_network->output->acti_vals[0];
			}
			existing_count++;
		}
		for (int h_index = 0; h_index < (int)this->consistency_explore_pre_obs.size(); h_index++) {
			vector<double> input = this->consistency_explore_pre_obs[h_index];
			input.insert(input.end(), this->consistency_explore_post_obs[h_index].begin(), this->consistency_explore_post_obs[h_index].end());

			this->consistency_network->activate(input);

			if (this->consistency_network->output->acti_vals[0] <= -3.0) {
				explore_sum_val += -3.0;
			} else {
				explore_sum_val += this->consistency_network->output->acti_vals[0];
			}
			explore_count++;
		}
		double existing_average = existing_sum_val / existing_count;
		cout << "existing_average: " << existing_average << endl;
		double explore_average = explore_sum_val / explore_count;
		cout << "explore_average: " << explore_average << endl;
	}
}

void Scope::update_signals() {
	// if (this->signal_pre_obs.size() > MIN_TRAIN_SAMPLES) {
	// 	cout << "this->id: " << this->id << endl;

	// 	if (this->pre_network == NULL) {
	// 		uniform_int_distribution<int> distribution(0, this->signal_pre_obs.size()-1);

	// 		this->pre_network = new Network(this->signal_pre_obs[0].size(),
	// 										NETWORK_SIZE_SMALL);
	// 		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
	// 			int index = distribution(generator);

	// 			this->pre_network->activate(this->signal_pre_obs[index]);

	// 			double error = this->signal_target_vals[index] - this->pre_network->output->acti_vals[0];

	// 			this->pre_network->backprop(error);
	// 		}

	// 		this->post_network = new Network(this->signal_pre_obs[0].size() + this->signal_post_obs[0].size(),
	// 										 NETWORK_SIZE_LARGE);
	// 		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
	// 			int index = distribution(generator);

	// 			vector<double> input = this->signal_pre_obs[index];
	// 			input.insert(input.end(), this->signal_post_obs[index].begin(), this->signal_post_obs[index].end());

	// 			this->post_network->activate(input);

	// 			double error = this->signal_target_vals[index] - this->post_network->output->acti_vals[0];

	// 			this->post_network->backprop(error);
	// 		}
	// 	} else {
	// 		uniform_int_distribution<int> distribution(0, this->signal_pre_obs.size()-1);

	// 		for (int iter_index = 0; iter_index < UPDATE_ITERS; iter_index++) {
	// 			int index = distribution(generator);

	// 			this->pre_network->activate(this->signal_pre_obs[index]);

	// 			double error = this->signal_target_vals[index] - this->pre_network->output->acti_vals[0];

	// 			this->pre_network->backprop(error);
	// 		}

	// 		for (int iter_index = 0; iter_index < UPDATE_ITERS; iter_index++) {
	// 			int index = distribution(generator);

	// 			vector<double> input = this->signal_pre_obs[index];
	// 			input.insert(input.end(), this->signal_post_obs[index].begin(), this->signal_post_obs[index].end());

	// 			this->post_network->activate(input);

	// 			double error = this->signal_target_vals[index] - this->post_network->output->acti_vals[0];

	// 			this->post_network->backprop(error);
	// 		}
	// 	}
	// }
}
