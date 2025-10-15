#include "refine_experiment.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#undef eigen_assert
#define eigen_assert(x) if (!(x)) {throw std::invalid_argument("Eigen error");}
#include <Eigen/Dense>

#include "globals.h"
#include "network.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int OPTIMIZE_ITERS = 10;
#else
const int OPTIMIZE_ITERS = 100000;
#endif /* MDEBUG */

void RefineExperiment::calc_vs() {
	Eigen::MatrixXd inputs(this->existing_target_vals.size(), 1 + this->existing_inputs.size());
	Eigen::VectorXd outputs(this->existing_target_vals.size());
	uniform_real_distribution<double> noise_distribution(-0.001, 0.001);
	/**
	 * - add some noise to prevent extremes
	 */
	for (int i_index = 0; i_index < (int)this->existing_target_vals.size(); i_index++) {
		inputs(i_index, 0) = 1.0;
		for (int f_index = 0; f_index < (int)this->existing_inputs.size(); f_index++) {
			inputs(i_index, 1 + f_index) = this->existing_factor_vals[i_index][f_index]
					+ noise_distribution(generator);
		}
		outputs(i_index) = this->existing_target_vals[i_index];
	}

	bool train_factor_success = true;

	Eigen::VectorXd weights;
	try {
		weights = inputs.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(outputs);
	} catch (std::invalid_argument &e) {
		cout << "Eigen error" << endl;
		train_factor_success = false;
	}

	#if defined(MDEBUG) && MDEBUG
	#else
	if (train_factor_success) {
		if (abs(weights(0)) > REGRESSION_WEIGHT_LIMIT) {
			cout << "abs(weights(0)): " << abs(weights(0)) << endl;
			train_factor_success = false;
		}
		for (int f_index = 0; f_index < (int)this->existing_inputs.size(); f_index++) {
			if (abs(weights(1 + f_index)) > REGRESSION_WEIGHT_LIMIT) {
				cout << "abs(weights(1 + f_index)): " << abs(weights(1 + f_index)) << endl;
				train_factor_success = false;
			}
		}
	}
	#endif /* MDEBUG */

	if (train_factor_success) {
		this->existing_constant = weights(0);
		for (int f_index = 0; f_index < (int)this->existing_inputs.size(); f_index++) {
			this->existing_weights[f_index] = weights(1 + f_index);
		}
	}

	vector<double> remaining_scores(this->existing_target_vals.size());
	for (int h_index = 0; h_index < (int)this->existing_target_vals.size(); h_index++) {
		double sum_score = this->existing_constant;
		for (int f_index = 0; f_index < (int)this->existing_inputs.size(); f_index++) {
			sum_score += this->existing_weights[f_index] * this->existing_factor_vals[h_index][f_index];
		}

		remaining_scores[h_index] = this->existing_target_vals[h_index] - sum_score;
	}

	if (this->existing_network != NULL) {
		uniform_int_distribution<int> input_distribution(0, this->existing_target_vals.size()-1);
		uniform_int_distribution<int> drop_distribution(0, 9);
		for (int iter_index = 0; iter_index < OPTIMIZE_ITERS; iter_index++) {
			int rand_index = input_distribution(generator);

			vector<bool> w_drop(this->existing_network_inputs.size());
			for (int i_index = 0; i_index < (int)this->existing_network_inputs.size(); i_index++) {
				if (drop_distribution(generator) == 0) {
					w_drop[i_index] = false;
				} else {
					w_drop[i_index] = this->existing_network_is_on[rand_index][i_index];
				}
			}

			this->existing_network->activate(this->existing_network_vals[rand_index],
											 w_drop);
			double error = remaining_scores[rand_index] - this->existing_network->output->acti_vals[0];
			this->existing_network->backprop(error);
		}
	}

	for (int h_index = 0; h_index < (int)this->new_target_vals.size(); h_index++) {
		if (this->new_target_val_status[h_index] == STATUS_TYPE_NEED_VS) {
			double existing_sum_vals = this->existing_constant;
			for (int i_index = 0; i_index < (int)this->existing_inputs.size(); i_index++) {
				existing_sum_vals += this->existing_weights[i_index] * this->new_existing_factor_vals[h_index][i_index];
			}
			if (this->existing_network != NULL) {
				this->existing_network->activate(this->new_existing_network_vals[h_index],
												 this->new_existing_network_is_on[h_index]);
				existing_sum_vals += this->existing_network->output->acti_vals[0];
			}

			this->new_target_vals[h_index] -= existing_sum_vals;

			this->new_target_val_status[h_index] = STATUS_TYPE_DONE;
		}
	}
}

bool RefineExperiment::refine() {
	Eigen::MatrixXd inputs(this->new_target_vals.size(), 1 + this->new_inputs.size());
	Eigen::VectorXd outputs(this->new_target_vals.size());
	uniform_real_distribution<double> noise_distribution(-0.001, 0.001);
	/**
	 * - add some noise to prevent extremes
	 */
	for (int i_index = 0; i_index < (int)this->new_target_vals.size(); i_index++) {
		inputs(i_index, 0) = 1.0;
		for (int f_index = 0; f_index < (int)this->new_inputs.size(); f_index++) {
			inputs(i_index, 1 + f_index) = this->new_factor_vals[i_index][f_index]
					+ noise_distribution(generator);
		}
		outputs(i_index) = this->new_target_vals[i_index];
	}

	bool train_factor_success = true;

	Eigen::VectorXd weights;
	try {
		weights = inputs.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(outputs);
	} catch (std::invalid_argument &e) {
		cout << "Eigen error" << endl;
		train_factor_success = false;
	}

	#if defined(MDEBUG) && MDEBUG
	#else
	if (train_factor_success) {
		if (abs(weights(0)) > REGRESSION_WEIGHT_LIMIT) {
			cout << "abs(weights(0)): " << abs(weights(0)) << endl;
			train_factor_success = false;
		}
		for (int f_index = 0; f_index < (int)this->new_inputs.size(); f_index++) {
			if (abs(weights(1 + f_index)) > REGRESSION_WEIGHT_LIMIT) {
				cout << "abs(weights(1 + f_index)): " << abs(weights(1 + f_index)) << endl;
				train_factor_success = false;
			}
		}
	}
	#endif /* MDEBUG */

	if (train_factor_success) {
		this->new_constant = weights(0);
		for (int f_index = 0; f_index < (int)this->new_inputs.size(); f_index++) {
			this->new_weights[f_index] = weights(1 + f_index);
		}
	}

	vector<double> sum_vals(this->new_target_vals.size());
	vector<double> remaining_scores(this->new_target_vals.size());
	for (int h_index = 0; h_index < (int)this->new_target_vals.size(); h_index++) {
		double sum_score = this->new_constant;
		for (int f_index = 0; f_index < (int)this->new_inputs.size(); f_index++) {
			sum_score += this->new_weights[f_index] * this->new_factor_vals[h_index][f_index];
		}

		sum_vals[h_index] = sum_score;
		remaining_scores[h_index] = this->new_target_vals[h_index] - sum_score;
	}

	if (this->new_network != NULL) {
		uniform_int_distribution<int> input_distribution(0, this->new_target_vals.size()-1);
		uniform_int_distribution<int> drop_distribution(0, 9);
		for (int iter_index = 0; iter_index < OPTIMIZE_ITERS; iter_index++) {
			int rand_index = input_distribution(generator);

			vector<bool> w_drop(this->new_network_inputs.size());
			for (int i_index = 0; i_index < (int)this->new_network_inputs.size(); i_index++) {
				if (drop_distribution(generator) == 0) {
					w_drop[i_index] = false;
				} else {
					w_drop[i_index] = this->new_network_is_on[rand_index][i_index];
				}
			}

			this->new_network->activate(this->new_network_vals[rand_index],
										w_drop);
			double error = remaining_scores[rand_index] - this->new_network->output->acti_vals[0];
			this->new_network->backprop(error);
		}

		for (int h_index = 0; h_index < (int)this->new_target_vals.size(); h_index++) {
			this->new_network->activate(this->new_network_vals[h_index],
										this->new_network_is_on[h_index]);
			sum_vals[h_index] += this->new_network->output->acti_vals[0];
		}
	}

	double sum_predicted_score = 0.0;
	for (int h_index = 0; h_index < (int)this->new_target_vals.size(); h_index++) {
		if (sum_vals[h_index] >= 0.0) {
			sum_predicted_score += this->new_target_vals[h_index];
		}
	}

	#if defined(MDEBUG) && MDEBUG
	if (rand()%2 == 0) {
		this->select_percentage = 0.5;
	} else {
		this->select_percentage = 0.0;
	}
	#else
	int num_positive = 0;
	for (int h_index = 0; h_index < (int)this->new_target_vals.size(); h_index++) {
		if (sum_vals[h_index] >= 0.0) {
			num_positive++;
		}
	}
	this->select_percentage = (double)num_positive / (double)this->new_target_vals.size();
	#endif /* MDEBUG */

	// // temp
	// cout << "refine" << endl;
	// cout << "sum_predicted_score: " << sum_predicted_score << endl;
	// cout << "this->select_percentage: " << this->select_percentage << endl;

	#if defined(MDEBUG) && MDEBUG
	if ((sum_predicted_score >= 0.0
			&& this->select_percentage > 0.0) || rand()%2 == 0) {
	#else
	if (sum_predicted_score >= 0.0
			&& this->select_percentage > 0.0) {
	#endif /* MDEBUG */
		return true;
	} else {
		return false;
	}
}
