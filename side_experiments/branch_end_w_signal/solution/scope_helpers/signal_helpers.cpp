/**
 * - train signals on signals
 *   - but eval experiments on true
 */

#include "scope.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "network.h"

using namespace std;

const int MIN_TRAIN_SIZE = 3;

void Scope::update_signals() {
	if (this->existing_pre_obs.size() > 0) {
		int max_sample_per_timestamp = (TOTAL_MAX_SAMPLES + (int)this->existing_pre_obs.size() - 1) / (int)this->existing_pre_obs.size();
		for (int t_index = 0; t_index < (int)this->existing_pre_obs.size(); t_index++) {
			while ((int)this->existing_pre_obs[t_index].size() > max_sample_per_timestamp) {
				uniform_int_distribution<int> distribution(0, this->existing_pre_obs[t_index].size()-1);
				int index = distribution(generator);
				this->existing_pre_obs[t_index].erase(this->existing_pre_obs[t_index].begin() + index);
				this->existing_post_obs[t_index].erase(this->existing_post_obs[t_index].begin() + index);
				this->existing_target_vals[t_index].erase(this->existing_target_vals[t_index].begin() + index);
			}

			while ((int)this->explore_pre_obs[t_index].size() > max_sample_per_timestamp) {
				uniform_int_distribution<int> distribution(0, this->explore_pre_obs[t_index].size()-1);
				int index = distribution(generator);
				this->explore_pre_obs[t_index].erase(this->explore_pre_obs[t_index].begin() + index);
				this->explore_post_obs[t_index].erase(this->explore_post_obs[t_index].begin() + index);
			}
		}
	}

	if (this->existing_pre_obs.size() >= MIN_TRAIN_SIZE) {
		if (this->consistency_network == NULL) {
			uniform_int_distribution<int> timestamp_distribution(0, this->existing_pre_obs.size()-1);

			this->consistency_network = new Network(this->existing_pre_obs[0][0].size() + this->existing_post_obs[0][0].size(),
													NETWORK_SIZE_LARGE);
			uniform_int_distribution<int> is_existing_distribution(0, 1);
			for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
				if (is_existing_distribution(generator) == 0) {
					int timestamp = timestamp_distribution(generator);
					uniform_int_distribution<int> distribution(0, this->existing_pre_obs[timestamp].size()-1);
					int index = distribution(generator);

					vector<double> input = this->existing_pre_obs[timestamp][index];
					input.insert(input.end(), this->existing_post_obs[timestamp][index].begin(), this->existing_post_obs[timestamp][index].end());

					this->consistency_network->activate(input);

					double error;
					if (this->consistency_network->output->acti_vals[0] >= 1.0) {
						error = 0.0;
					} else {
						error = 1.0 - this->consistency_network->output->acti_vals[0];
					}

					this->consistency_network->backprop(error);
				} else {
					int timestamp = timestamp_distribution(generator);
					if (this->explore_pre_obs[timestamp].size() > 0) {
						uniform_int_distribution<int> distribution(0, this->explore_pre_obs[timestamp].size()-1);
						int index = distribution(generator);

						vector<double> input = this->explore_pre_obs[timestamp][index];
						input.insert(input.end(), this->explore_post_obs[timestamp][index].begin(), this->explore_post_obs[timestamp][index].end());

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

			// // temp
			// double existing_sum_val = 0.0;
			// int existing_count = 0;
			// double explore_sum_val = 0.0;
			// int explore_count = 0;
			// for (int t_index = 0; t_index < (int)this->existing_pre_obs.size(); t_index++) {
			// 	for (int h_index = 0; h_index < (int)this->existing_pre_obs[t_index].size(); h_index++) {
			// 		vector<double> input = this->existing_pre_obs[t_index][h_index];
			// 		input.insert(input.end(), this->existing_post_obs[t_index][h_index].begin(), this->existing_post_obs[t_index][h_index].end());

			// 		this->consistency_network->activate(input);

			// 		existing_sum_val += this->consistency_network->output->acti_vals[0];
			// 		existing_count++;
			// 	}

			// 	for (int h_index = 0; h_index < (int)this->explore_pre_obs[t_index].size(); h_index++) {
			// 		vector<double> input = this->explore_pre_obs[t_index][h_index];
			// 		input.insert(input.end(), this->explore_post_obs[t_index][h_index].begin(), this->explore_post_obs[t_index][h_index].end());

			// 		this->consistency_network->activate(input);

			// 		explore_sum_val += this->consistency_network->output->acti_vals[0];
			// 		explore_count++;
			// 	}
			// }
			// double existing_average = existing_sum_val / existing_count;
			// cout << "existing_average: " << existing_average << endl;
			// double explore_average = explore_sum_val / explore_count;
			// cout << "explore_average: " << explore_average << endl;

			this->pre_network = new Network(this->existing_pre_obs[0][0].size(),
											NETWORK_SIZE_SMALL);
			for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
				int timestamp = timestamp_distribution(generator);
				uniform_int_distribution<int> distribution(0, this->existing_pre_obs[timestamp].size()-1);
				int index = distribution(generator);

				this->pre_network->activate(this->existing_pre_obs[timestamp][index]);

				double error = this->existing_target_vals[timestamp][index] - this->pre_network->output->acti_vals[0];

				this->pre_network->backprop(error);
			}

			this->post_network = new Network(this->existing_pre_obs[0][0].size() + this->existing_post_obs[0][0].size(),
											 NETWORK_SIZE_LARGE);
			for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
				int timestamp = timestamp_distribution(generator);
				uniform_int_distribution<int> distribution(0, this->existing_pre_obs[timestamp].size()-1);
				int index = distribution(generator);

				vector<double> input = this->existing_pre_obs[timestamp][index];
				input.insert(input.end(), this->existing_post_obs[timestamp][index].begin(), this->existing_post_obs[timestamp][index].end());

				this->post_network->activate(input);

				double error = this->existing_target_vals[timestamp][index] - this->post_network->output->acti_vals[0];

				this->post_network->backprop(error);
			}
		} else {
			uniform_int_distribution<int> timestamp_distribution(0, this->existing_pre_obs.size()-1);

			uniform_int_distribution<int> is_existing_distribution(0, 1);
			for (int iter_index = 0; iter_index < UPDATE_ITERS; iter_index++) {
				if (is_existing_distribution(generator) == 0) {
					int timestamp = timestamp_distribution(generator);
					uniform_int_distribution<int> distribution(0, this->existing_pre_obs[timestamp].size()-1);
					int index = distribution(generator);

					vector<double> input = this->existing_pre_obs[timestamp][index];
					input.insert(input.end(), this->existing_post_obs[timestamp][index].begin(), this->existing_post_obs[timestamp][index].end());

					this->consistency_network->activate(input);

					double error;
					if (this->consistency_network->output->acti_vals[0] >= 1.0) {
						error = 0.0;
					} else {
						error = 1.0 - this->consistency_network->output->acti_vals[0];
					}

					this->consistency_network->backprop(error);
				} else {
					int timestamp = timestamp_distribution(generator);
					if (this->explore_pre_obs[timestamp].size() > 0) {
						uniform_int_distribution<int> distribution(0, this->explore_pre_obs[timestamp].size()-1);
						int index = distribution(generator);

						vector<double> input = this->explore_pre_obs[timestamp][index];
						input.insert(input.end(), this->explore_post_obs[timestamp][index].begin(), this->explore_post_obs[timestamp][index].end());

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

			for (int iter_index = 0; iter_index < UPDATE_ITERS; iter_index++) {
				int timestamp = timestamp_distribution(generator);
				uniform_int_distribution<int> distribution(0, this->existing_pre_obs[timestamp].size()-1);
				int index = distribution(generator);

				this->pre_network->activate(this->existing_pre_obs[timestamp][index]);

				double error = this->existing_target_vals[timestamp][index] - this->pre_network->output->acti_vals[0];

				this->pre_network->backprop(error);
			}

			for (int iter_index = 0; iter_index < UPDATE_ITERS; iter_index++) {
				int timestamp = timestamp_distribution(generator);
				uniform_int_distribution<int> distribution(0, this->existing_pre_obs[timestamp].size()-1);
				int index = distribution(generator);

				vector<double> input = this->existing_pre_obs[timestamp][index];
				input.insert(input.end(), this->existing_post_obs[timestamp][index].begin(), this->existing_post_obs[timestamp][index].end());

				this->post_network->activate(input);

				double error = this->existing_target_vals[timestamp][index] - this->post_network->output->acti_vals[0];

				this->post_network->backprop(error);
			}
		}
	}

	this->existing_pre_obs.push_back(vector<vector<double>>());
	this->existing_post_obs.push_back(vector<vector<double>>());
	this->existing_target_vals.push_back(vector<double>());

	this->explore_pre_obs.push_back(vector<vector<double>>());
	this->explore_post_obs.push_back(vector<vector<double>>());
}
