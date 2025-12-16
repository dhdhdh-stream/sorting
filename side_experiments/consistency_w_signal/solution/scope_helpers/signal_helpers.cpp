#include "scope.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "network.h"

using namespace std;

void Scope::update_signals() {
	if (this->signal_status != SIGNAL_STATUS_FAIL
			&& this->existing_pre_obs.size() >= MIN_TRAIN_SAMPLES
			&& this->explore_pre_obs.size() >= MIN_TRAIN_SAMPLES) {
		cout << "update_signals this->id: " << this->id << endl;

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

		vector<double> consistency_vals(this->explore_pre_obs.size());
		vector<vector<double>> meaningful_pre_obs;
		vector<vector<double>> meaningful_post_obs;
		vector<double> meaningful_target_vals;
		for (int h_index = 0; h_index < (int)this->explore_pre_obs.size(); h_index++) {
			vector<double> input = this->explore_pre_obs[h_index];
			input.insert(input.end(), this->explore_post_obs[h_index].begin(), this->explore_post_obs[h_index].end());

			this->consistency_network->activate(input);

			double consistency = this->consistency_network->output->acti_vals[0];
			if (consistency > 0.0) {
				if (consistency > 1.0) {
					consistency = 1.0;
				}

				meaningful_pre_obs.push_back(this->explore_pre_obs[h_index]);
				meaningful_post_obs.push_back(this->explore_post_obs[h_index]);
				meaningful_target_vals.push_back(this->explore_target_vals[h_index]);
			}
		}

		// temp
		cout << "this->explore_pre_obs.size(): " << this->explore_pre_obs.size() << endl;
		cout << "meaningful_pre_obs.size(): " << meaningful_pre_obs.size() << endl;

		uniform_int_distribution<int> distribution(0, meaningful_pre_obs.size()-1);
		if (this->signal_network == NULL) {
			this->signal_network = new Network(this->explore_pre_obs[0].size() + this->explore_post_obs[0].size(),
											   NETWORK_SIZE_LARGE);

			for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
				int index = distribution(generator);

				double consistency = consistency_vals[index];
				if (consistency > 0.0) {
					vector<double> input = meaningful_pre_obs[index];
					input.insert(input.end(), meaningful_post_obs[index].begin(), meaningful_post_obs[index].end());

					this->signal_network->activate(input);

					double post_error = meaningful_target_vals[index] - this->signal_network->output->acti_vals[0];
					post_error *= consistency;

					this->signal_network->backprop(post_error);
				}
			}

			this->signal_status = SIGNAL_STATUS_VALID;
		} else {
			for (int iter_index = 0; iter_index < UPDATE_ITERS; iter_index++) {
				int index = distribution(generator);

				double consistency = consistency_vals[index];
				if (consistency > 0.0) {
					vector<double> input = meaningful_pre_obs[index];
					input.insert(input.end(), meaningful_post_obs[index].begin(), meaningful_post_obs[index].end());

					this->signal_network->activate(input);

					double post_error = meaningful_target_vals[index] - this->signal_network->output->acti_vals[0];
					post_error *= consistency;

					this->signal_network->backprop(post_error);
				}
			}
		}
	}
}

// temp
void Scope::measure_signal_pcc() {
	if (this->explore_signals.size() > 0) {
		double sum_signal_vals = 0.0;
		for (int s_index = 0; s_index < (int)this->explore_signals.size(); s_index++) {
			sum_signal_vals += this->explore_signals[s_index];
		}
		double signal_val_average = sum_signal_vals / (double)this->explore_signals.size();

		double sum_signal_variance = 0.0;
		for (int s_index = 0; s_index < (int)this->explore_signals.size(); s_index++) {
			sum_signal_variance += (this->explore_signals[s_index] - signal_val_average)
				* (this->explore_signals[s_index] - signal_val_average);
		}
		double signal_val_standard_deviation = sqrt(sum_signal_variance / (double)this->explore_signals.size());

		double sum_true_vals = 0.0;
		for (int s_index = 0; s_index < (int)this->explore_true.size(); s_index++) {
			sum_true_vals += this->explore_true[s_index];
		}
		double true_val_average = sum_true_vals / (double)this->explore_true.size();

		double sum_true_variance = 0.0;
		for (int s_index = 0; s_index < (int)this->explore_true.size(); s_index++) {
			sum_true_variance += (this->explore_true[s_index] - true_val_average)
				* (this->explore_true[s_index] - true_val_average);
		}
		double true_val_standard_deviation = sqrt(sum_true_variance / (double)this->explore_true.size());

		double sum_covariance = 0.0;
		for (int s_index = 0; s_index < (int)this->explore_signals.size(); s_index++) {
			sum_covariance += (this->explore_signals[s_index] - signal_val_average)
				* (this->explore_true[s_index] - true_val_average);
		}
		double covariance_average = sum_covariance / (double)this->explore_signals.size();

		double pcc = covariance_average / signal_val_standard_deviation / true_val_standard_deviation;

		cout << this->id << " pcc: " << pcc << endl;
	}

	this->explore_signals.clear();
	this->explore_true.clear();
}
