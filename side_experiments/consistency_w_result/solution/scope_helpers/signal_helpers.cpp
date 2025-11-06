#include "scope.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "network.h"

using namespace std;

void Scope::update_signals() {
	if (this->explore_pre_obs.back().size() > 0) {
		cout << "this->id: " << this->id << endl;

		if (this->consistency_network == NULL) {
			this->consistency_network = new Network(this->existing_pre_obs[0][0].size() + this->existing_post_obs[0][0].size(),
													NETWORK_SIZE_LARGE);
			uniform_int_distribution<int> is_existing_distribution(0, 1);
			for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
				if (is_existing_distribution(generator) == 0) {
					uniform_int_distribution<int> distribution(0, this->existing_pre_obs[0].size()-1);
					int index = distribution(generator);

					vector<double> input = this->existing_pre_obs[0][index];
					input.insert(input.end(), this->existing_post_obs[0][index].begin(), this->existing_post_obs[0][index].end());

					this->consistency_network->activate(input);

					double error;
					if (this->consistency_network->output->acti_vals[0] >= 1.0) {
						error = 0.0;
					} else {
						error = 1.0 - this->consistency_network->output->acti_vals[0];
					}

					this->consistency_network->backprop(error);
				} else {
					uniform_int_distribution<int> distribution(0, this->explore_pre_obs[0].size()-1);
					int index = distribution(generator);

					vector<double> input = this->explore_pre_obs[0][index];
					input.insert(input.end(), this->explore_post_obs[0][index].begin(), this->explore_post_obs[0][index].end());

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
			uniform_int_distribution<int> focus_new_distribution(0, 9);
			uniform_int_distribution<int> timestamp_distribution(0, this->existing_pre_obs.size()-1);
			uniform_int_distribution<int> is_existing_distribution(0, 1);
			for (int epoch_iter = 0; epoch_iter < 15; epoch_iter++) {
				for (int iter_index = 0; iter_index < 10000; iter_index++) {
					if (is_existing_distribution(generator) == 0) {
						int timestamp;
						if (focus_new_distribution(generator) - 5 - epoch_iter >= 0) {
							timestamp = (int)this->existing_pre_obs.size()-1;
						} else {
							timestamp = timestamp_distribution(generator);
						}

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
						int timestamp;
						if (focus_new_distribution(generator) - 5 - epoch_iter >= 0) {
							timestamp = (int)this->existing_pre_obs.size()-1;
						} else {
							timestamp = timestamp_distribution(generator);
						}

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
		}

		double existing_sum_val = 0.0;
		int existing_count = 0;
		double explore_sum_val = 0.0;
		int explore_count = 0;
		for (int t_index = 0; t_index < (int)this->existing_pre_obs.size(); t_index++) {
			for (int h_index = 0; h_index < (int)this->existing_pre_obs[t_index].size(); h_index++) {
				vector<double> input = this->existing_pre_obs[t_index][h_index];
				input.insert(input.end(), this->existing_post_obs[t_index][h_index].begin(), this->existing_post_obs[t_index][h_index].end());

				this->consistency_network->activate(input);

				if (this->consistency_network->output->acti_vals[0] >= 1.0) {
					existing_sum_val += 1.0;
				} else {
					existing_sum_val += this->consistency_network->output->acti_vals[0];
				}
				existing_count++;
			}

			for (int h_index = 0; h_index < (int)this->explore_pre_obs[t_index].size(); h_index++) {
				vector<double> input = this->explore_pre_obs[t_index][h_index];
				input.insert(input.end(), this->explore_post_obs[t_index][h_index].begin(), this->explore_post_obs[t_index][h_index].end());

				this->consistency_network->activate(input);

				if (this->consistency_network->output->acti_vals[0] <= -1.0) {
					explore_sum_val += -1.0;
				} else {
					explore_sum_val += this->consistency_network->output->acti_vals[0];
				}
				explore_count++;
			}
		}
		double existing_average = existing_sum_val / existing_count;
		cout << "existing_average: " << existing_average << endl;
		double explore_average = explore_sum_val / explore_count;
		cout << "explore_average: " << explore_average << endl;

		int max_sample_per_timestamp = (TOTAL_MAX_SAMPLES + (int)this->existing_pre_obs.size() - 1) / (int)this->existing_pre_obs.size();
		for (int t_index = 0; t_index < (int)this->existing_pre_obs.size(); t_index++) {
			while ((int)this->existing_pre_obs[t_index].size() > max_sample_per_timestamp) {
				uniform_int_distribution<int> distribution(0, this->existing_pre_obs[t_index].size()-1);
				int index = distribution(generator);
				this->existing_pre_obs[t_index].erase(this->existing_pre_obs[t_index].begin() + index);
				this->existing_post_obs[t_index].erase(this->existing_post_obs[t_index].begin() + index);
			}

			while ((int)this->explore_pre_obs[t_index].size() > max_sample_per_timestamp) {
				uniform_int_distribution<int> distribution(0, this->explore_pre_obs[t_index].size()-1);
				int index = distribution(generator);
				this->explore_pre_obs[t_index].erase(this->explore_pre_obs[t_index].begin() + index);
				this->explore_post_obs[t_index].erase(this->explore_post_obs[t_index].begin() + index);
			}
		}

		this->existing_pre_obs.push_back(vector<vector<double>>());
		this->existing_post_obs.push_back(vector<vector<double>>());

		this->explore_pre_obs.push_back(vector<vector<double>>());
		this->explore_post_obs.push_back(vector<vector<double>>());
	}
}
