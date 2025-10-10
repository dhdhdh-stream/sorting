/**
 * - if predicted good, but actual bad, then training samples must not be representative
 *   - which may be corrected with additional samples from measure
 *     - so retry until success or predicted becomes bad
 * 
 * - weigh explore vs. measure 50/50 for both factor and network
 */

#include "branch_experiment.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#undef eigen_assert
#define eigen_assert(x) if (!(x)) {throw std::invalid_argument("Eigen error");}
#include <Eigen/Dense>

#include "globals.h"
#include "network.h"
#include "solution_helpers.h"

using namespace std;

const double SEED_RATIO = 0.2;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_ITERS = 30;
#else
const int TRAIN_ITERS = 300000;
#endif /* MDEBUG */

bool BranchExperiment::retrain_helper() {
	int num_explore_seeds = SEED_RATIO * (double)this->explore_scope_histories.size();
	vector<int> explore_seed_indexes;
	vector<int> explore_remaining_indexes(this->explore_scope_histories.size());
	for (int i_index = 0; i_index < (int)this->explore_scope_histories.size(); i_index++) {
		explore_remaining_indexes.push_back(i_index);
	}
	for (int s_index = 0; s_index < num_explore_seeds; s_index++) {
		uniform_int_distribution<int> distribution(0, explore_remaining_indexes.size()-1);
		int index = distribution(generator);
		explore_seed_indexes.push_back(explore_remaining_indexes[index]);
		explore_remaining_indexes.erase(explore_remaining_indexes.begin() + index);
	}

	int num_measure_seeds = SEED_RATIO * (double)this->measure_scope_histories.size();
	vector<int> measure_seed_indexes;
	vector<int> measure_remaining_indexes(this->measure_scope_histories.size());
	for (int i_index = 0; i_index < (int)this->measure_scope_histories.size(); i_index++) {
		measure_remaining_indexes.push_back(i_index);
	}
	for (int s_index = 0; s_index < num_measure_seeds; s_index++) {
		uniform_int_distribution<int> distribution(0, measure_remaining_indexes.size()-1);
		int index = distribution(generator);
		measure_seed_indexes.push_back(measure_remaining_indexes[index]);
		measure_remaining_indexes.erase(measure_remaining_indexes.begin() + index);
	}

	vector<vector<double>> explore_factor_normalized_vals(this->explore_scope_histories.size());
	for (int h_index = 0; h_index < (int)this->explore_scope_histories.size(); h_index++) {
		vector<double> curr_vals(this->new_inputs.size());
		for (int i_index = 0; i_index < (int)this->new_inputs.size(); i_index++) {
			double val;
			bool is_on;
			fetch_input_helper(this->explore_scope_histories[h_index],
							   this->new_inputs[i_index],
							   0,
							   val,
							   is_on);
			if (is_on) {
				double normalized_val = (val - this->new_input_averages[i_index]) / this->new_input_standard_deviations[i_index];
				curr_vals[i_index] = normalized_val;
			} else {
				curr_vals[i_index] = 0.0;
			}
		}
		explore_factor_normalized_vals[h_index] = curr_vals;
	}

	vector<vector<double>> measure_factor_normalized_vals(this->measure_scope_histories.size());
	for (int h_index = 0; h_index < (int)this->measure_scope_histories.size(); h_index++) {
		vector<double> curr_vals(this->new_inputs.size());
		for (int i_index = 0; i_index < (int)this->new_inputs.size(); i_index++) {
			double val;
			bool is_on;
			fetch_input_helper(this->measure_scope_histories[h_index],
							   this->new_inputs[i_index],
							   0,
							   val,
							   is_on);
			if (is_on) {
				double normalized_val = (val - this->new_input_averages[i_index]) / this->new_input_standard_deviations[i_index];
				curr_vals[i_index] = normalized_val;
			} else {
				curr_vals[i_index] = 0.0;
			}
		}
		measure_factor_normalized_vals[h_index] = curr_vals;
	}

	if (this->new_inputs.size() > 0) {
		Eigen::MatrixXd inputs(2 * measure_remaining_indexes.size(), 1 + this->new_inputs.size());
		Eigen::VectorXd outputs(2 * measure_remaining_indexes.size());
		uniform_real_distribution<double> noise_distribution(-0.001, 0.001);
		/**
		 * - add some noise to prevent extremes
		 */
		uniform_int_distribution<int> explore_distribution(0, explore_remaining_indexes.size()-1);
		for (int i_index = 0; i_index < (int)measure_remaining_indexes.size(); i_index++) {
			int index = explore_distribution(generator);
			inputs(i_index, 0) = 1.0;
			for (int f_index = 0; f_index < (int)this->new_inputs.size(); f_index++) {
				inputs(i_index, 1 + f_index) = explore_factor_normalized_vals[explore_remaining_indexes[index]][f_index]
					+ noise_distribution(generator);
			}
			outputs(i_index) = this->explore_target_val_histories[explore_remaining_indexes[index]];
		}
		for (int i_index = 0; i_index < (int)measure_remaining_indexes.size(); i_index++) {
			inputs(measure_remaining_indexes.size() + i_index, 0) = 1.0;
			for (int f_index = 0; f_index < (int)this->new_inputs.size(); f_index++) {
				inputs(measure_remaining_indexes.size() + i_index, 1 + f_index) =
					measure_factor_normalized_vals[measure_remaining_indexes[i_index]][f_index]
						+ noise_distribution(generator);
			}
			outputs(measure_remaining_indexes.size() + i_index) = this->measure_target_val_histories[measure_remaining_indexes[i_index]];
		}

		Eigen::VectorXd weights;
		try {
			weights = inputs.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(outputs);
		} catch (std::invalid_argument &e) {
			cout << "Eigen error" << endl;
			return false;
		}

		#if defined(MDEBUG) && MDEBUG
		#else
		if (abs(weights(0)) > REGRESSION_WEIGHT_LIMIT) {
			cout << "abs(weights(0)): " << abs(weights(0)) << endl;
			return false;
		}
		for (int f_index = 0; f_index < (int)factor_inputs.size(); f_index++) {
			if (abs(weights(1 + f_index)) > REGRESSION_WEIGHT_LIMIT) {
				cout << "abs(weights(1 + f_index)): " << abs(weights(1 + f_index)) << endl;
				return false;
			}
		}
		#endif /* MDEBUG */

		/**
		 * - assume train factor always reasonable due to additional samples
		 */
		this->new_constant = weights(0);
		for (int f_index = 0; f_index < (int)this->new_inputs.size(); f_index++) {
			this->new_weights[f_index] = weights(1 + f_index);
		}
	}

	vector<double> explore_sum_vals(this->explore_scope_histories.size());
	vector<double> explore_remaining_scores(this->explore_scope_histories.size());
	for (int h_index = 0; h_index < (int)this->explore_scope_histories.size(); h_index++) {
		double sum_score = this->new_constant;
		for (int f_index = 0; f_index < (int)this->new_inputs.size(); f_index++) {
			sum_score += this->new_weights[f_index]
				* explore_factor_normalized_vals[h_index][f_index];
		}

		explore_sum_vals[h_index] = sum_score;
		explore_remaining_scores[h_index] = this->explore_target_val_histories[h_index] - sum_score;
	}

	vector<double> measure_sum_vals(this->measure_scope_histories.size());
	vector<double> measure_remaining_scores(this->measure_scope_histories.size());
	for (int h_index = 0; h_index < (int)this->measure_scope_histories.size(); h_index++) {
		double sum_score = this->new_constant;
		for (int f_index = 0; f_index < (int)this->new_inputs.size(); f_index++) {
			sum_score += this->new_weights[f_index]
				* measure_factor_normalized_vals[h_index][f_index];
		}

		measure_sum_vals[h_index] = sum_score;
		measure_remaining_scores[h_index] = this->measure_target_val_histories[h_index] - sum_score;
	}

	vector<vector<double>> explore_vals(this->explore_scope_histories.size());
	vector<vector<bool>> explore_is_on(this->explore_scope_histories.size());
	for (int h_index = 0; h_index < (int)this->explore_scope_histories.size(); h_index++) {
		vector<double> curr_vals(this->new_network_inputs.size());
		vector<bool> curr_is_on(this->new_network_inputs.size());
		for (int i_index = 0; i_index < (int)this->new_network_inputs.size(); i_index++) {
			double val;
			bool is_on;
			fetch_input_helper(this->explore_scope_histories[h_index],
							   this->new_network_inputs[i_index],
							   0,
							   val,
							   is_on);
			curr_vals[i_index] = val;
			curr_is_on[i_index] = is_on;
		}
		explore_vals[h_index] = curr_vals;
		explore_is_on[h_index] = curr_is_on;
	}

	vector<vector<double>> measure_vals(this->measure_scope_histories.size());
	vector<vector<bool>> measure_is_on(this->measure_scope_histories.size());
	for (int h_index = 0; h_index < (int)this->measure_scope_histories.size(); h_index++) {
		vector<double> curr_vals(this->new_network_inputs.size());
		vector<bool> curr_is_on(this->new_network_inputs.size());
		for (int i_index = 0; i_index < (int)this->new_network_inputs.size(); i_index++) {
			double val;
			bool is_on;
			fetch_input_helper(this->measure_scope_histories[h_index],
							   this->new_network_inputs[i_index],
							   0,
							   val,
							   is_on);
			curr_vals[i_index] = val;
			curr_is_on[i_index] = is_on;
		}
		measure_vals[h_index] = curr_vals;
		measure_is_on[h_index] = curr_is_on;
	}

	Network* network = new Network((int)this->new_network_inputs.size(),
								   this->new_network->input_averages,
								   this->new_network->input_standard_deviations);

	uniform_int_distribution<int> is_explore_distribution(0, 1);
	uniform_int_distribution<int> explore_input_distribution(0, explore_remaining_indexes.size()-1);
	uniform_int_distribution<int> measure_input_distribution(0, measure_remaining_indexes.size()-1);
	uniform_int_distribution<int> drop_distribution(0, 9);
	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		bool is_explore = is_explore_distribution(generator) == 0;
		int rand_index;
		if (is_explore) {
			rand_index = explore_remaining_indexes[explore_input_distribution(generator)];
		} else {
			rand_index = measure_remaining_indexes[measure_input_distribution(generator)];
		}

		vector<bool> w_drop(this->new_network_inputs.size());
		for (int i_index = 0; i_index < (int)this->new_network_inputs.size(); i_index++) {
			if (drop_distribution(generator) == 0) {
				w_drop[i_index] = false;
			} else {
				if (is_explore) {
					w_drop[i_index] = explore_is_on[rand_index][i_index];
				} else {
					w_drop[i_index] = measure_is_on[rand_index][i_index];
				}
			}
		}

		if (is_explore) {
			network->activate(explore_vals[rand_index],
							  w_drop);
			double error = explore_remaining_scores[rand_index] - network->output->acti_vals[0];
			network->backprop(error);
		} else {
			network->activate(measure_vals[rand_index],
							  w_drop);
			double error = measure_remaining_scores[rand_index] - network->output->acti_vals[0];
			network->backprop(error);
		}
	}

	vector<double> explore_network_vals(this->explore_scope_histories.size());
	for (int h_index = 0; h_index < (int)this->explore_scope_histories.size(); h_index++) {
		network->activate(explore_vals[h_index],
						  explore_is_on[h_index]);

		explore_network_vals[h_index] = network->output->acti_vals[0];
	}

	vector<double> measure_network_vals(this->measure_scope_histories.size());
	for (int h_index = 0; h_index < (int)this->measure_scope_histories.size(); h_index++) {
		network->activate(measure_vals[h_index],
						  measure_is_on[h_index]);

		measure_network_vals[h_index] = network->output->acti_vals[0];
	}

	double sum_explore_default_misguess = 0.0;
	for (int s_index = 0; s_index < (int)explore_seed_indexes.size(); s_index++) {
		sum_explore_default_misguess += explore_remaining_scores[explore_seed_indexes[s_index]]
			* explore_remaining_scores[explore_seed_indexes[s_index]];
	}

	double sum_explore_network_misguess = 0.0;
	for (int s_index = 0; s_index < (int)explore_seed_indexes.size(); s_index++) {
		sum_explore_network_misguess += (explore_remaining_scores[explore_seed_indexes[s_index]] - explore_network_vals[explore_seed_indexes[s_index]])
			* (explore_remaining_scores[explore_seed_indexes[s_index]] - explore_network_vals[explore_seed_indexes[s_index]]);
	}

	double sum_measure_default_misguess = 0.0;
	for (int s_index = 0; s_index < (int)measure_seed_indexes.size(); s_index++) {
		sum_measure_default_misguess += measure_remaining_scores[measure_seed_indexes[s_index]]
			* measure_remaining_scores[measure_seed_indexes[s_index]];
	}

	double sum_measure_network_misguess = 0.0;
	for (int s_index = 0; s_index < (int)measure_seed_indexes.size(); s_index++) {
		sum_measure_network_misguess += (measure_remaining_scores[measure_seed_indexes[s_index]] - measure_network_vals[measure_seed_indexes[s_index]])
			* (measure_remaining_scores[measure_seed_indexes[s_index]] - measure_network_vals[measure_seed_indexes[s_index]]);
	}

	#if defined(MDEBUG) && MDEBUG
	if ((sum_explore_network_misguess < sum_explore_default_misguess
				&& sum_measure_network_misguess < sum_measure_default_misguess)
			|| rand()%4 != 0) {
	#else
	if (sum_explore_network_misguess < sum_explore_default_misguess
			&& sum_measure_network_misguess < sum_measure_default_misguess) {
	#endif /* MDEBUG */
		for (int h_index = 0; h_index < (int)this->explore_scope_histories.size(); h_index++) {
			explore_sum_vals[h_index] += explore_network_vals[h_index];
		}
		for (int h_index = 0; h_index < (int)this->measure_scope_histories.size(); h_index++) {
			measure_sum_vals[h_index] += measure_network_vals[h_index];
		}

		if (this->new_network != NULL) {
			delete this->new_network;
		}
		this->new_network = network;
	} else {
		delete network;

		this->new_network_inputs.clear();
		if (this->new_network != NULL) {
			delete this->new_network;
		}
		this->new_network = NULL;
	}

	double seed_explore_sum_predicted_score = 0.0;
	for (int s_index = 0; s_index < (int)explore_seed_indexes.size(); s_index++) {
		if (explore_sum_vals[explore_seed_indexes[s_index]] >= 0.0) {
			seed_explore_sum_predicted_score += explore_target_val_histories[explore_seed_indexes[s_index]];
		}
	}

	double explore_sum_predicted_score = 0.0;
	for (int h_index = 0; h_index < (int)this->explore_scope_histories.size(); h_index++) {
		if (explore_sum_vals[h_index] >= 0.0) {
			explore_sum_predicted_score += this->explore_target_val_histories[h_index];
		}
	}

	double seed_measure_sum_predicted_score = 0.0;
	for (int s_index = 0; s_index < (int)measure_seed_indexes.size(); s_index++) {
		if (measure_sum_vals[measure_seed_indexes[s_index]] >= 0.0) {
			seed_measure_sum_predicted_score += measure_target_val_histories[measure_seed_indexes[s_index]];
		}
	}

	double measure_sum_predicted_score = 0.0;
	for (int h_index = 0; h_index < (int)this->measure_scope_histories.size(); h_index++) {
		if (measure_sum_vals[h_index] >= 0.0) {
			measure_sum_predicted_score += this->measure_target_val_histories[h_index];
		}
	}

	int num_positive = 0;
	for (int h_index = 0; h_index < (int)this->explore_scope_histories.size(); h_index++) {
		if (explore_sum_vals[h_index] >= 0.0) {
			num_positive++;
		}
	}
	for (int h_index = 0; h_index < (int)this->measure_scope_histories.size(); h_index++) {
		if (measure_sum_vals[h_index] >= 0.0) {
			num_positive++;
		}
	}

	if (seed_explore_sum_predicted_score >= 0.0
			&& explore_sum_predicted_score >= 0.0
			&& seed_measure_sum_predicted_score >= 0.0
			&& measure_sum_predicted_score >= 0.0
			&& num_positive > 0) {
		return true;
	} else {
		return false;
	}
}
