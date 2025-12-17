#include "scope.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "network.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

const int MIN_EVAL_ITERS = 3;

void Scope::update_signals(SolutionWrapper* wrapper) {
	switch (this->signal_status) {
	case SIGNAL_STATUS_INIT:
		if (this->existing_pre_obs.size() >= MIN_TRAIN_SAMPLES
				&& this->explore_pre_obs.size() >= MIN_TRAIN_SAMPLES) {
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

			this->signal_network = new Network(this->explore_pre_obs[0].size() + this->explore_post_obs[0].size(),
											   NETWORK_SIZE_LARGE);

			uniform_int_distribution<int> distribution(0, meaningful_pre_obs.size()-1);
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

			this->signal_status = SIGNAL_STATUS_TEST;
		}
		break;
	case SIGNAL_STATUS_TEST:
		/**
		 * - check that signal is increasing and correlates with result
		 */
		if (this->signal_history.size() >= MIN_EVAL_ITERS) {
			vector<double> iter_vals(this->signal_history.size());
			for (int i_index = 0; i_index < (int)this->signal_history.size(); i_index++) {
				iter_vals[i_index] = i_index;
			}
			double increase_pcc = calc_pcc(this->signal_history,
										   iter_vals);
			cout << "increase_pcc: " << increase_pcc << endl;

			vector<double> score_vals(this->signal_history.size());
			for (int i_index = 0; i_index < (int)this->signal_history.size(); i_index++) {
				score_vals[i_index] = wrapper->solution->improvement_history[
					wrapper->solution->improvement_history.size() - this->signal_history.size() + i_index];
			}
			double correlation_pcc = calc_pcc(this->signal_history,
											  score_vals);
			cout << "correlation_pcc: " << correlation_pcc << endl;
		}
		break;
	}
}
