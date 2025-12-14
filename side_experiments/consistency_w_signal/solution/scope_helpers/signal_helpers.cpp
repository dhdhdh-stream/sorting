#include "scope.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "network.h"

using namespace std;

void Scope::update_signals() {
	if (this->signal_status != SIGNAL_STATUS_FAIL
			&& this->existing_pre_obs.size() >= MIN_TRAIN_SAMPLES
			&& this->explore_pre_obs.size() > MIN_TRAIN_SAMPLES) {
		cout << "this->id: " << this->id << endl;

		if (this->consistency_network == NULL) {
			this->consistency_network = new Network(this->existing_pre_obs[0].size() + this->existing_post_obs[0].size(),
													NETWORK_SIZE_LARGE);
			uniform_int_distribution<int> is_existing_distribution(0, 1);
			uniform_int_distribution<int> existing_distribution(0, this->existing_pre_obs.size()-1);
			uniform_int_distribution<int> explore_distribution(0, this->explore_pre_obs.size()-1);
			for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
				if (is_existing_distribution(generator) == 0) {
					int index = existing_distribution(generator);

					vector<double> input = this->existing_pre_obs[index];
					input.insert(input.end(), this->existing_post_obs[index].begin(), this->existing_post_obs[index].end());

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

					vector<double> input = this->explore_pre_obs[index];
					input.insert(input.end(), this->explore_post_obs[index].begin(), this->explore_post_obs[index].end());

					this->consistency_network->activate(input);

					double error;
					if (this->consistency_network->output->acti_vals[0] <= 0.0) {
						error = 0.0;
					} else {
						error = 0.0 - this->consistency_network->output->acti_vals[0];
					}

					this->consistency_network->backprop(error);
				}
			}
		} else {
			uniform_int_distribution<int> is_existing_distribution(0, 1);
			uniform_int_distribution<int> existing_distribution(0, this->existing_pre_obs.size()-1);
			uniform_int_distribution<int> explore_distribution(0, this->explore_pre_obs.size()-1);
			for (int iter_index = 0; iter_index < UPDATE_ITERS; iter_index++) {
				if (is_existing_distribution(generator) == 0) {
					int index = existing_distribution(generator);

					vector<double> input = this->existing_pre_obs[index];
					input.insert(input.end(), this->existing_post_obs[index].begin(), this->existing_post_obs[index].end());

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

					vector<double> input = this->explore_pre_obs[index];
					input.insert(input.end(), this->explore_post_obs[index].begin(), this->explore_post_obs[index].end());

					this->consistency_network->activate(input);

					double error;
					if (this->consistency_network->output->acti_vals[0] <= 0.0) {
						error = 0.0;
					} else {
						error = 0.0 - this->consistency_network->output->acti_vals[0];
					}

					this->consistency_network->backprop(error);
				}
			}
		}

		if (this->pre_network == NULL) {
			uniform_int_distribution<int> distribution(0, this->explore_pre_obs.size()-1);

			this->pre_network = new Network(this->explore_pre_obs[0].size(),
											NETWORK_SIZE_SMALL);
			this->post_network = new Network(this->explore_pre_obs[0].size() + this->explore_post_obs[0].size(),
											 NETWORK_SIZE_LARGE);

			for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
				int index = distribution(generator);

				vector<double> input = this->explore_pre_obs[index];
				input.insert(input.end(), this->explore_post_obs[index].begin(), this->explore_post_obs[index].end());

				this->consistency_network->activate(input);

				double consistency = this->consistency_network->output->acti_vals[0];
				if (consistency > 0.0) {
					if (consistency > 1.0) {
						consistency = 1.0;
					}

					this->pre_network->activate(this->explore_pre_obs[index]);

					double pre_error = this->explore_target_vals[index] - this->pre_network->output->acti_vals[0];
					pre_error *= consistency;

					this->pre_network->backprop(pre_error);

					this->post_network->activate(input);

					double post_error = this->explore_target_vals[index] - this->post_network->output->acti_vals[0];
					post_error *= consistency;

					this->post_network->backprop(post_error);
				}
			}

			this->signal_status = SIGNAL_STATUS_VALID;
		} else {
			uniform_int_distribution<int> distribution(0, this->explore_pre_obs.size()-1);

			for (int iter_index = 0; iter_index < UPDATE_ITERS; iter_index++) {
				int index = distribution(generator);

				vector<double> input = this->explore_pre_obs[index];
				input.insert(input.end(), this->explore_post_obs[index].begin(), this->explore_post_obs[index].end());

				this->consistency_network->activate(input);

				double consistency = this->consistency_network->output->acti_vals[0];
				if (consistency > 0.0) {
					if (consistency > 1.0) {
						consistency = 1.0;
					}

					this->pre_network->activate(this->explore_pre_obs[index]);

					double pre_error = this->explore_target_vals[index] - this->pre_network->output->acti_vals[0];
					pre_error *= consistency;

					this->pre_network->backprop(pre_error);

					this->post_network->activate(input);

					double post_error = this->explore_target_vals[index] - this->post_network->output->acti_vals[0];
					post_error *= consistency;

					this->post_network->backprop(post_error);
				}
			}
		}
	}
}
