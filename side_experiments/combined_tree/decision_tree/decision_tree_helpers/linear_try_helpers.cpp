#include "decision_tree_helpers.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#undef eigen_assert
#define eigen_assert(x) if (!(x)) {throw std::invalid_argument("Eigen error");}
#include <Eigen/Dense>

#include "constants.h"
#include "decision_tree.h"
#include "globals.h"

using namespace std;

void linear_try(vector<vector<double>>& train_obs_histories,
				vector<double>& train_previous_val_histories,
				vector<double>& train_target_val_histories,
				double& curr_constant,
				vector<int>& curr_input_indexes,
				vector<double>& curr_input_weights,
				double& curr_previous_weight,
				vector<std::vector<double>>& test_obs_histories,
				vector<double>& test_previous_val_histories,
				vector<double>& test_target_val_histories,
				double& curr_sum_misguess) {
	vector<int> remaining_indexes(train_obs_histories[0].size());
	for (int i_index = 0; i_index < (int)train_obs_histories[0].size(); i_index++) {
		remaining_indexes[i_index] = i_index;
	}

	while (true) {
		if (remaining_indexes.size() == 0) {
			break;
		}

		uniform_int_distribution<int> distribution(0, remaining_indexes.size()-1);
		int index = distribution(generator);

		curr_input_indexes.push_back(remaining_indexes[index]);

		remaining_indexes.erase(remaining_indexes.begin() + index);

		if (curr_input_indexes.size() >= DT_NODE_MAX_NUM_INPUTS) {
			break;
		}
	}

	Eigen::MatrixXd inputs(train_obs_histories.size(), 1 + curr_input_indexes.size() + 1);
	uniform_real_distribution<double> noise_distribution(-0.001, 0.001);
	/**
	 * - add some noise to prevent extremes
	 */
	for (int h_index = 0; h_index < (int)train_obs_histories.size(); h_index++) {
		inputs(h_index, 0) = 1.0;
		for (int i_index = 0; i_index < (int)curr_input_indexes.size(); i_index++) {
			inputs(h_index, 1 + i_index) = train_obs_histories[h_index][curr_input_indexes[i_index]];
		}
		inputs(h_index, 1 + curr_input_indexes.size()) = train_previous_val_histories[h_index];
	}

	Eigen::VectorXd outputs(train_obs_histories.size());
	for (int h_index = 0; h_index < (int)train_obs_histories.size(); h_index++) {
		outputs(h_index) = train_target_val_histories[h_index];
	}

	Eigen::VectorXd weights;
	try {
		weights = inputs.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(outputs);
	} catch (std::invalid_argument &e) {
		cout << "Eigen error" << endl;
		curr_sum_misguess = numeric_limits<double>::max();
		return;
	}

	#if defined(MDEBUG) && MDEBUG
	#else
	if (abs(weights(0)) > REGRESSION_WEIGHT_LIMIT) {
		cout << "abs(weights(0)): " << abs(weights(0)) << endl;
		curr_sum_misguess = numeric_limits<double>::max();
		return;
	}
	for (int i_index = 0; i_index < (int)curr_input_indexes.size(); i_index++) {
		if (abs(weights(1 + i_index)) > REGRESSION_WEIGHT_LIMIT) {
			cout << "abs(weights(1 + i_index)): " << abs(weights(1 + i_index)) << endl;
			curr_sum_misguess = numeric_limits<double>::max();
			return;
		}
	}
	if (abs(weights(1 + curr_input_indexes.size())) > REGRESSION_WEIGHT_LIMIT) {
		cout << "abs(weights(1 + curr_input_indexes.size())): " << abs(weights(1 + curr_input_indexes.size())) << endl;
		curr_sum_misguess = numeric_limits<double>::max();
		return;
	}
	#endif /* MDEBUG */

	curr_constant = weights(0);
	curr_input_weights = vector<double>(curr_input_indexes.size());
	for (int i_index = 0; i_index < (int)curr_input_indexes.size(); i_index++) {
		curr_input_weights[i_index] = weights(1 + i_index);
	}
	curr_previous_weight = weights(1 + curr_input_indexes.size());

	curr_sum_misguess = 0.0;
	for (int h_index = 0; h_index < (int)test_obs_histories.size(); h_index++) {
		double sum_vals = curr_constant;
		for (int i_index = 0; i_index < (int)curr_input_indexes.size(); i_index++) {
			sum_vals += curr_input_weights[i_index] * test_obs_histories[h_index][curr_input_indexes[i_index]];
		}
		sum_vals += curr_previous_weight * test_previous_val_histories[h_index];

		curr_sum_misguess += (test_target_val_histories[h_index] - sum_vals)
			* (test_target_val_histories[h_index] - sum_vals);
	}
}
