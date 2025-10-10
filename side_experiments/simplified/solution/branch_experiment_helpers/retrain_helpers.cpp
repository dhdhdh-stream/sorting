/**
 * - if predicted good, but actual bad, then training samples must not be representative
 *   - which may be corrected with additional samples from measure
 *     - so retry until success or predicted becomes bad
 * 
 * - simply have explore vs. measure weight 50/50
 */

// TODO: measure num samples may be less than explore num samples
// TODO: explore may also be overly lucky

#include "branch_experiment.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#undef eigen_assert
#define eigen_assert(x) if (!(x)) {throw std::invalid_argument("Eigen error");}
#include <Eigen/Dense>

#include "constants.h"
#include "globals.h"
#include "network.h"
#include "solution_helpers.h"

using namespace std;

const double SEED_RATIO = 0.3;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_ITERS = 30;
#else
const int TRAIN_ITERS = 300000;
#endif /* MDEBUG */

bool BranchExperiment::retrain_helper() {
	double potential_constant;
	vector<double> potential_factor_weights;
	Network* potential_network = NULL;

	int num_explore_seeds = SEED_RATIO * (double)this->explore_scope_histories.size();
	vector<int> explore_seed_indexes;
	vector<int> explore_remaining_indexes(this->explore_scope_histories.size());
	for (int i_index = 0; i_index < (int)this->explore_scope_histories.size(); i_index++) {
		explore_remaining_indexes[i_index] = i_index;
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
		measure_remaining_indexes[i_index] = i_index;
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
		for (int f_index = 0; f_index < (int)this->new_inputs.size(); f_index++) {
			if (abs(weights(1 + f_index)) > REGRESSION_WEIGHT_LIMIT) {
				cout << "abs(weights(1 + f_index)): " << abs(weights(1 + f_index)) << endl;
				return false;
			}
		}
		#endif /* MDEBUG */

		/**
		 * - assume train factor always reasonable due to additional samples
		 */
		potential_constant = weights(0);
		for (int f_index = 0; f_index < (int)this->new_inputs.size(); f_index++) {
			potential_factor_weights.push_back(weights(1 + f_index));
		}
	} else {
		double sum_explore_vals = 0.0;
		for (int h_index = 0; h_index < (int)this->explore_scope_histories.size(); h_index++) {
			sum_explore_vals += this->explore_target_val_histories[h_index];
		}
		double explore_val_average = sum_explore_vals / (double)this->explore_scope_histories.size();

		double sum_measure_vals = 0.0;
		for (int h_index = 0; h_index < (int)this->measure_scope_histories.size(); h_index++) {
			sum_measure_vals += this->measure_target_val_histories[h_index];
		}
		double measure_val_average = sum_measure_vals / (double)this->measure_scope_histories.size();

		potential_constant = (explore_val_average + measure_val_average) / 2.0;
	}

	vector<double> explore_sum_vals(this->explore_scope_histories.size());
	vector<double> explore_remaining_scores(this->explore_scope_histories.size());
	for (int h_index = 0; h_index < (int)this->explore_scope_histories.size(); h_index++) {
		double sum_score = potential_constant;
		for (int f_index = 0; f_index < (int)this->new_inputs.size(); f_index++) {
			sum_score += potential_factor_weights[f_index]
				* explore_factor_normalized_vals[h_index][f_index];
		}

		explore_sum_vals[h_index] = sum_score;
		explore_remaining_scores[h_index] = this->explore_target_val_histories[h_index] - sum_score;
	}

	vector<double> measure_sum_vals(this->measure_scope_histories.size());
	vector<double> measure_remaining_scores(this->measure_scope_histories.size());
	for (int h_index = 0; h_index < (int)this->measure_scope_histories.size(); h_index++) {
		double sum_score = potential_constant;
		for (int f_index = 0; f_index < (int)this->new_inputs.size(); f_index++) {
			sum_score += potential_factor_weights[f_index]
				* measure_factor_normalized_vals[h_index][f_index];
		}

		measure_sum_vals[h_index] = sum_score;
		measure_remaining_scores[h_index] = this->measure_target_val_histories[h_index] - sum_score;
	}

	if (this->new_network != NULL) {
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
			if (is_explore_distribution(generator) == 0) {
				int rand_index = explore_remaining_indexes[explore_input_distribution(generator)];

				vector<bool> w_drop(this->new_network_inputs.size());
				for (int i_index = 0; i_index < (int)this->new_network_inputs.size(); i_index++) {
					if (drop_distribution(generator) == 0) {
						w_drop[i_index] = false;
					} else {
						w_drop[i_index] = explore_is_on[rand_index][i_index];
					}
				}

				network->activate(explore_vals[rand_index],
								  w_drop);
				double error = explore_remaining_scores[rand_index] - network->output->acti_vals[0];
				network->backprop(error);
			} else {
				int rand_index = measure_remaining_indexes[measure_input_distribution(generator)];

				vector<bool> w_drop(this->new_network_inputs.size());
				for (int i_index = 0; i_index < (int)this->new_network_inputs.size(); i_index++) {
					if (drop_distribution(generator) == 0) {
						w_drop[i_index] = false;
					} else {
						w_drop[i_index] = measure_is_on[rand_index][i_index];
					}
				}

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
		double explore_default_misguess_average = sum_explore_default_misguess / (double)explore_seed_indexes.size();

		double sum_explore_network_misguess = 0.0;
		for (int s_index = 0; s_index < (int)explore_seed_indexes.size(); s_index++) {
			sum_explore_network_misguess += (explore_remaining_scores[explore_seed_indexes[s_index]] - explore_network_vals[explore_seed_indexes[s_index]])
				* (explore_remaining_scores[explore_seed_indexes[s_index]] - explore_network_vals[explore_seed_indexes[s_index]]);
		}
		double explore_network_misguess_average = sum_explore_network_misguess / (double)explore_seed_indexes.size();

		double sum_measure_default_misguess = 0.0;
		for (int s_index = 0; s_index < (int)measure_seed_indexes.size(); s_index++) {
			sum_measure_default_misguess += measure_remaining_scores[measure_seed_indexes[s_index]]
				* measure_remaining_scores[measure_seed_indexes[s_index]];
		}
		double measure_default_misguess_average = sum_measure_default_misguess / (double)measure_seed_indexes.size();

		double sum_measure_network_misguess = 0.0;
		for (int s_index = 0; s_index < (int)measure_seed_indexes.size(); s_index++) {
			sum_measure_network_misguess += (measure_remaining_scores[measure_seed_indexes[s_index]] - measure_network_vals[measure_seed_indexes[s_index]])
				* (measure_remaining_scores[measure_seed_indexes[s_index]] - measure_network_vals[measure_seed_indexes[s_index]]);
		}
		double measure_network_misguess_average = sum_measure_network_misguess / (double)measure_seed_indexes.size();

		double combined_default_misguess_average = (explore_default_misguess_average + measure_default_misguess_average) / 2.0;
		double combined_network_misguess_average = (explore_network_misguess_average + measure_network_misguess_average) / 2.0;

		cout << "combined_default_misguess_average: " << combined_default_misguess_average << endl;
		cout << "combined_network_misguess_average: " << combined_network_misguess_average << endl;

		#if defined(MDEBUG) && MDEBUG
		if (combined_network_misguess_average < combined_default_misguess_average
				|| rand()%4 != 0) {
		#else
		if (combined_network_misguess_average < combined_default_misguess_average) {
		#endif /* MDEBUG */
			for (int h_index = 0; h_index < (int)this->explore_scope_histories.size(); h_index++) {
				explore_sum_vals[h_index] += explore_network_vals[h_index];
			}
			for (int h_index = 0; h_index < (int)this->measure_scope_histories.size(); h_index++) {
				measure_sum_vals[h_index] += measure_network_vals[h_index];
			}

			potential_network = network;
		} else {
			delete network;
		}
	}

	double seed_explore_sum_predicted_score = 0.0;
	for (int s_index = 0; s_index < (int)explore_seed_indexes.size(); s_index++) {
		if (explore_sum_vals[explore_seed_indexes[s_index]] >= 0.0) {
			seed_explore_sum_predicted_score += this->explore_target_val_histories[explore_seed_indexes[s_index]];
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
			seed_measure_sum_predicted_score += this->measure_target_val_histories[measure_seed_indexes[s_index]];
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

	// temp
	cout << "retrain" << endl;
	cout << "seed_explore_sum_predicted_score: " << seed_explore_sum_predicted_score << endl;
	cout << "explore_sum_predicted_score: " << explore_sum_predicted_score << endl;
	cout << "seed_measure_sum_predicted_score: " << seed_measure_sum_predicted_score << endl;
	cout << "measure_sum_predicted_score: " << measure_sum_predicted_score << endl;
	cout << "num_positive: " << num_positive << endl;

	#if defined(MDEBUG) && MDEBUG
	if ((seed_explore_sum_predicted_score >= 0.0
			&& explore_sum_predicted_score >= 0.0
			&& seed_measure_sum_predicted_score >= 0.0
			&& measure_sum_predicted_score >= 0.0
			&& num_positive > 0) || rand()%2 == 0) {
	#else
	if (seed_explore_sum_predicted_score >= 0.0
			&& explore_sum_predicted_score >= 0.0
			&& seed_measure_sum_predicted_score >= 0.0
			&& measure_sum_predicted_score >= 0.0
			&& num_positive > 0) {
	#endif /* MDEBUG */
		this->new_constant = potential_constant;
		this->new_weights = potential_factor_weights;
		if (this->new_network != NULL) {
			delete this->new_network;
		}
		this->new_network = potential_network;

		return true;
	} else {
		if (potential_network != NULL) {
			delete potential_network;
		}

		return false;
	}
}
