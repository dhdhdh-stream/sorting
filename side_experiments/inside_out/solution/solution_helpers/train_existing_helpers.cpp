/**
 * - if change has a 1% impact, need ~50000 samples to confirm
 *   - ultimately, key is still reward signals
 */

#include "solution_helpers.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#undef eigen_assert
#define eigen_assert(x) if (!(x)) {throw std::invalid_argument("Eigen error");}
#include <Eigen/Dense>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "factor.h"
#include "globals.h"
#include "network.h"
#include "nn_helpers.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_helpers.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int EXISTING_GATHER_NUM_SAMPLES = 2;
#else
const int EXISTING_GATHER_NUM_SAMPLES = 40;
#endif /* MDEBUG */

const int EXISTING_NUM_FACTORS = 40;

bool train_existing(vector<ScopeHistory*>& scope_histories,
					vector<double>& target_val_histories,
					double& average_score,
					vector<Input>& factor_inputs,
					vector<double>& factor_input_averages,
					vector<double>& factor_input_standard_deviations,
					vector<double>& factor_weights) {
	Scope* scope = scope_histories[0]->scope;

	int num_instances = (int)target_val_histories.size();

	map<Input, InputData> input_tracker;

	vector<vector<double>> a_factor_vals(scope->factors.size());
	vector<double> a_factor_averages(scope->factors.size());
	vector<double> a_factor_standard_deviations(scope->factors.size());
	vector<int> remaining_factors;
	for (int f_index = 0; f_index < (int)scope->factors.size(); f_index++) {
		Input input;
		input.scope_context = {scope};
		input.factor_index = f_index;
		input.node_context = {-1};
		input.obs_index = -1;

		InputData input_data;
		analyze_input(input,
					  scope_histories,
					  input_data);
		input_tracker[input] = input_data;

		if (input_data.standard_deviation >= MIN_STANDARD_DEVIATION) {
			vector<double> curr_factor_vals(num_instances);
			for (int h_index = 0; h_index < num_instances; h_index++) {
				double val;
				bool is_on;
				fetch_input_helper(scope_histories[h_index],
								   input,
								   0,
								   val,
								   is_on);
				if (is_on) {
					double normalized_val = (val - input_data.average) / input_data.standard_deviation;
					curr_factor_vals[h_index] = normalized_val;
				} else {
					curr_factor_vals[h_index] = 0.0;
				}
			}

			a_factor_vals[f_index] = curr_factor_vals;

			double sum_vals = 0.0;
			for (int h_index = 0; h_index < num_instances; h_index++) {
				sum_vals += curr_factor_vals[h_index];
			}
			a_factor_averages[f_index] = sum_vals / (double)num_instances;

			double sum_variances = 0.0;
			for (int h_index = 0; h_index < num_instances; h_index++) {
				sum_variances += (curr_factor_vals[h_index] - a_factor_averages[f_index])
					* (curr_factor_vals[h_index] - a_factor_averages[f_index]);
			}
			a_factor_standard_deviations[f_index] = sqrt(sum_variances / (double)num_instances);

			remaining_factors.push_back(f_index);
		}
	}

	vector<vector<double>> factor_vals;
	vector<double> h_factor_averages;
	vector<double> h_factor_standard_deviations;
	while (remaining_factors.size() > 0) {
		uniform_int_distribution<int> random_distribution(0, remaining_factors.size()-1);
		int random_index = random_distribution(generator);
		int factor_index = remaining_factors[random_index];

		bool should_add = true;
		for (int e_index = 0; e_index < (int)factor_vals.size(); e_index++) {
			double sum_covariance = 0.0;
			for (int h_index = 0; h_index < num_instances; h_index++) {
				sum_covariance += (factor_vals[e_index][h_index] - h_factor_averages[e_index])
					* (a_factor_vals[factor_index][h_index] - a_factor_averages[factor_index]);
			}
			double covariance = sum_covariance / (double)num_instances;

			double pcc = covariance / h_factor_standard_deviations[e_index] / a_factor_standard_deviations[factor_index];
			if (abs(pcc) > UNIQUE_MAX_PCC) {
				should_add = false;
				break;
			}
		}

		if (should_add) {
			Input input;
			input.scope_context = {scope};
			input.factor_index = factor_index;
			input.node_context = {-1};
			input.obs_index = -1;

			InputData input_data = input_tracker[input];

			factor_inputs.push_back(input);
			factor_input_averages.push_back(input_data.average);
			factor_input_standard_deviations.push_back(input_data.standard_deviation);

			factor_vals.push_back(a_factor_vals[factor_index]);
			h_factor_averages.push_back(a_factor_averages[factor_index]);
			h_factor_standard_deviations.push_back(a_factor_standard_deviations[factor_index]);

			if (factor_inputs.size() >= EXISTING_NUM_FACTORS) {
				break;
			}
		}

		remaining_factors.erase(remaining_factors.begin() + random_index);
	}

	Eigen::MatrixXd inputs(num_instances, 1 + factor_inputs.size());
	uniform_real_distribution<double> noise_distribution(-0.001, 0.001);
	/**
	 * - add some noise to prevent extremes
	 */
	for (int i_index = 0; i_index < num_instances; i_index++) {
		inputs(i_index, 0) = 1.0;
		for (int f_index = 0; f_index < (int)factor_inputs.size(); f_index++) {
			inputs(i_index, 1 + f_index) = factor_vals[f_index][i_index] + noise_distribution(generator);
		}
	}

	Eigen::VectorXd outputs(num_instances);
	for (int i_index = 0; i_index < num_instances; i_index++) {
		outputs(i_index) = target_val_histories[i_index];
	}

	Eigen::VectorXd weights;
	try {
		weights = inputs.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(outputs);
	} catch (std::invalid_argument &e) {
		cout << "Eigen error" << endl;
		return false;
	}

	if (abs(weights(0)) > REGRESSION_WEIGHT_LIMIT) {
		cout << "abs(weights(0)): " << abs(weights(0)) << endl;
		return false;
	}
	average_score = weights(0);
	for (int f_index = 0; f_index < (int)factor_inputs.size(); f_index++) {
		if (abs(weights(1 + f_index)) > REGRESSION_WEIGHT_LIMIT) {
			cout << "abs(weights(1 + f_index)): " << abs(weights(1 + f_index)) << endl;
			return false;
		}
		factor_weights.push_back(weights(1 + f_index));
	}

	#if defined(MDEBUG) && MDEBUG
	#else
	Eigen::VectorXd predicted = inputs * weights;
	double sum_offset = 0.0;
	for (int i_index = 0; i_index < num_instances; i_index++) {
		sum_offset += abs(predicted(i_index) - average_score);
	}
	double average_offset = sum_offset / (double)num_instances;
	double impact_threshold = average_offset * FACTOR_IMPACT_THRESHOLD;
	#endif /* MDEBUG */

	for (int f_index = (int)factor_inputs.size() - 1; f_index >= 0; f_index--) {
		#if defined(MDEBUG) && MDEBUG
		if (rand()%2 == 0) {
		#else
		double sum_impact = 0.0;
		for (int i_index = 0; i_index < num_instances; i_index++) {
			sum_impact += abs(inputs(i_index, 1 + f_index));
		}

		double impact = abs(factor_weights[f_index]) * sum_impact / num_instances;
		if (impact < impact_threshold) {
		#endif /* MDEBUG */
			factor_inputs.erase(factor_inputs.begin() + f_index);
			factor_input_averages.erase(factor_input_averages.begin() + f_index);
			factor_input_standard_deviations.erase(factor_input_standard_deviations.begin() + f_index);
			factor_weights.erase(factor_weights.begin() + f_index);

			factor_vals.erase(factor_vals.begin() + f_index);
		}
	}

	vector<double> remaining_scores(num_instances);
	for (int i_index = 0; i_index < num_instances; i_index++) {
		double sum_score = 0.0;
		for (int f_index = 0; f_index < (int)factor_inputs.size(); f_index++) {
			sum_score += factor_weights[f_index] * factor_vals[f_index][i_index];
		}

		remaining_scores[i_index] = target_val_histories[i_index]
			- average_score - sum_score;
	}

	vector<int> best_indexes(EXISTING_GATHER_NUM_SAMPLES, -1);
	vector<double> best_scores(EXISTING_GATHER_NUM_SAMPLES);
	for (int h_index = 0; h_index < num_instances; h_index++) {
		if (best_indexes.back() == -1
				|| remaining_scores[h_index] > best_scores.back()) {
			best_indexes.back() = h_index;
			best_scores.back() = remaining_scores[h_index];

			int curr_index = EXISTING_GATHER_NUM_SAMPLES - 2;
			while (true) {
				if (curr_index < 0) {
					break;
				}

				if (best_indexes[curr_index] == -1
						|| best_scores[curr_index + 1] > best_scores[curr_index]) {
					double temp_index = best_indexes[curr_index];
					double temp_score = best_scores[curr_index];
					best_indexes[curr_index] = best_indexes[curr_index + 1];
					best_scores[curr_index] = best_scores[curr_index + 1];
					best_indexes[curr_index + 1] = temp_index;
					best_scores[curr_index + 1] = temp_score;

					curr_index--;
				} else {
					break;
				}
			}
		}
	}

	map<Input, double> best_t_scores;
	for (int i_index = 0; i_index < (int)best_indexes.size(); i_index++) {
		vector<Scope*> scope_context;
		vector<int> node_context;
		gather_t_scores_helper(scope_histories[best_indexes[i_index]],
							   scope_context,
							   node_context,
							   best_t_scores,
							   scope_histories,
							   input_tracker);
	}

	vector<int> worst_indexes(EXISTING_GATHER_NUM_SAMPLES, -1);
	vector<double> worst_scores(EXISTING_GATHER_NUM_SAMPLES);
	for (int h_index = 0; h_index < num_instances; h_index++) {
		if (worst_indexes.back() == -1
				|| target_val_histories[h_index] < worst_scores.back()) {
			worst_indexes.back() = h_index;
			worst_scores.back() = target_val_histories[h_index];

			int curr_index = EXISTING_GATHER_NUM_SAMPLES - 2;
			while (true) {
				if (curr_index < 0) {
					break;
				}

				if (worst_indexes[curr_index] == -1
						|| worst_scores[curr_index + 1] < worst_scores[curr_index]) {
					double temp_index = worst_indexes[curr_index];
					double temp_score = worst_scores[curr_index];
					worst_indexes[curr_index] = worst_indexes[curr_index + 1];
					worst_scores[curr_index] = worst_scores[curr_index + 1];
					worst_indexes[curr_index + 1] = temp_index;
					worst_scores[curr_index + 1] = temp_score;

					curr_index--;
				} else {
					break;
				}
			}
		}
	}

	map<Input, double> worst_t_scores;
	for (int i_index = 0; i_index < (int)worst_indexes.size(); i_index++) {
		vector<Scope*> scope_context;
		vector<int> node_context;
		gather_t_scores_helper(scope_histories[worst_indexes[i_index]],
							   scope_context,
							   node_context,
							   worst_t_scores,
							   scope_histories,
							   input_tracker);
	}

	map<Input, double> contrast_t_scores;
	for (map<Input, double>::iterator best_it = best_t_scores.begin();
			best_it != best_t_scores.end(); best_it++) {
		map<Input, double>::iterator worst_it = worst_t_scores.find(best_it->first);
		if (worst_it == worst_t_scores.end()) {
			contrast_t_scores[best_it->first] = abs(best_it->second);
		} else {
			contrast_t_scores[best_it->first] = abs(best_it->second - worst_it->second);
		}
	}
	for (map<Input, double>::iterator worst_it = worst_t_scores.begin();
			worst_it != worst_t_scores.end(); worst_it++) {
		map<Input, double>::iterator best_it = best_t_scores.find(worst_it->first);
		if (best_it == best_t_scores.end()) {
			contrast_t_scores[worst_it->first] = abs(worst_it->second);
		}
	}

	vector<pair<double,Input>> s_contrast_t_scores;
	for (map<Input, double>::iterator it = contrast_t_scores.begin();
			it != contrast_t_scores.end(); it++) {
		s_contrast_t_scores.push_back({it->second, it->first});
	}
	sort(s_contrast_t_scores.begin(), s_contrast_t_scores.end());

	vector<Input> network_inputs;
	vector<vector<double>> v_input_vals;
	vector<vector<bool>> v_input_is_on;
	vector<vector<double>> n_input_vals;
	vector<double> h_averages;
	vector<double> h_standard_deviations;
	for (int i_index = (int)s_contrast_t_scores.size()-1; i_index >= 0; i_index--) {
		Input input = s_contrast_t_scores[i_index].second;
		InputData input_data = input_tracker[input];

		vector<double> curr_input_vals(num_instances);
		vector<bool> curr_input_is_on(num_instances);
		vector<double> curr_n_input_vals(num_instances);
		double curr_average = input_data.average;
		double curr_standard_deviation = input_data.standard_deviation;
		for (int h_index = 0; h_index < num_instances; h_index++) {
			double val;
			bool is_on;
			fetch_input_helper(scope_histories[h_index],
							   input,
							   0,
							   val,
							   is_on);
			curr_input_vals[h_index] = val;
			curr_input_is_on[h_index] = is_on;
			if (is_on) {
				double normalized_val = (val - curr_average) / curr_standard_deviation;
				curr_n_input_vals[h_index] = normalized_val;
			} else {
				curr_n_input_vals[h_index] = 0.0;
			}
		}

		double potential_average;
		double potential_standard_deviation;
		bool should_add = is_unique(n_input_vals,
									h_averages,
									h_standard_deviations,
									curr_n_input_vals,
									potential_average,
									potential_standard_deviation);

		s_contrast_t_scores.pop_back();

		if (should_add) {
			network_inputs.push_back(input);
			v_input_vals.push_back(curr_input_vals);
			v_input_is_on.push_back(curr_input_is_on);

			n_input_vals.push_back(curr_n_input_vals);
			h_averages.push_back(potential_average);
			h_standard_deviations.push_back(potential_standard_deviation);

			if (network_inputs.size() >= INPUT_NUM_HIGHEST) {
				break;
			}
		}
	}
	while (s_contrast_t_scores.size() > 0) {
		uniform_int_distribution<int> input_distribution(0, s_contrast_t_scores.size()-1);
		int input_index = input_distribution(generator);

		Input input = s_contrast_t_scores[input_index].second;
		InputData input_data = input_tracker[input];

		vector<double> curr_input_vals(num_instances);
		vector<bool> curr_input_is_on(num_instances);
		vector<double> curr_n_input_vals(num_instances);
		double curr_average = input_data.average;
		double curr_standard_deviation = input_data.standard_deviation;
		for (int h_index = 0; h_index < num_instances; h_index++) {
			double val;
			bool is_on;
			fetch_input_helper(scope_histories[h_index],
							   input,
							   0,
							   val,
							   is_on);
			curr_input_vals[h_index] = val;
			curr_input_is_on[h_index] = is_on;
			if (is_on) {
				double normalized_val = (val - curr_average) / curr_standard_deviation;
				curr_n_input_vals[h_index] = normalized_val;
			} else {
				curr_n_input_vals[h_index] = 0.0;
			}
		}

		double potential_average;
		double potential_standard_deviation;
		bool should_add = is_unique(n_input_vals,
									h_averages,
									h_standard_deviations,
									curr_n_input_vals,
									potential_average,
									potential_standard_deviation);

		s_contrast_t_scores.erase(s_contrast_t_scores.begin() + input_index);

		if (should_add) {
			network_inputs.push_back(input);
			v_input_vals.push_back(curr_input_vals);
			v_input_is_on.push_back(curr_input_is_on);

			n_input_vals.push_back(curr_n_input_vals);
			h_averages.push_back(potential_average);
			h_standard_deviations.push_back(potential_standard_deviation);

			if (network_inputs.size() >= INPUT_NUM_HIGHEST + INPUT_NUM_RANDOM) {
				break;
			}
		}
	}

	vector<vector<double>> input_vals(num_instances);
	vector<vector<bool>> input_is_on(num_instances);
	for (int h_index = 0; h_index < num_instances; h_index++) {
		vector<double> curr_input_vals(network_inputs.size());
		vector<bool> curr_input_is_on(network_inputs.size());
		for (int i_index = 0; i_index < (int)network_inputs.size(); i_index++) {
			curr_input_vals[i_index] = v_input_vals[i_index][h_index];
			curr_input_is_on[i_index] = v_input_is_on[i_index][h_index];
		}
		input_vals[h_index] = curr_input_vals;
		input_is_on[h_index] = curr_input_is_on;
	}

	double sum_misguess = 0.0;
	for (int i_index = 0; i_index < num_instances; i_index++) {
		sum_misguess += remaining_scores[i_index] * remaining_scores[i_index];
	}
	double average_misguess = sum_misguess / num_instances;

	double sum_misguess_variance = 0.0;
	for (int i_index = 0; i_index < num_instances; i_index++) {
		double curr_misguess = remaining_scores[i_index] * remaining_scores[i_index];
		sum_misguess_variance += (curr_misguess - average_misguess) * (curr_misguess - average_misguess);
	}
	double misguess_standard_deviation = sqrt(sum_misguess_variance / num_instances);
	if (misguess_standard_deviation < MIN_STANDARD_DEVIATION) {
		misguess_standard_deviation = MIN_STANDARD_DEVIATION;
	}

	Network* new_network = new Network((int)network_inputs.size(),
									   input_vals,
									   input_is_on);

	train_network(input_vals,
				  input_is_on,
				  remaining_scores,
				  new_network);

	vector<double> network_vals(num_instances);
	for (int i_index = 0; i_index < num_instances; i_index++) {
		new_network->activate(input_vals[i_index],
							  input_is_on[i_index]);
		network_vals[i_index] = new_network->output->acti_vals[0];
	}

	double sum_network_vals = 0.0;
	for (int i_index = 0; i_index < num_instances; i_index++) {
		sum_network_vals += network_vals[i_index];
	}
	double network_average = sum_network_vals / (double)num_instances;

	double sum_variances = 0.0;
	for (int i_index = 0; i_index < num_instances; i_index++) {
		sum_variances += (network_vals[i_index] - network_average)
			* (network_vals[i_index] - network_average);
	}
	double network_standard_deviation = sqrt(sum_variances / (double)num_instances);

	bool should_add = true;
	for (int f_index = 0; f_index < (int)scope->factors.size(); f_index++) {
		if (a_factor_vals[f_index].size() > 0) {
			double sum_covariance = 0.0;
			for (int h_index = 0; h_index < num_instances; h_index++) {
				sum_covariance += (a_factor_vals[f_index][h_index] - a_factor_averages[f_index])
					* (network_vals[h_index] - network_average);
			}
			double covariance = sum_covariance / (double)num_instances;

			double pcc = covariance / a_factor_standard_deviations[f_index] / network_standard_deviation;
			if (abs(pcc) > UNIQUE_MAX_PCC) {
				should_add = false;
				break;
			}
		}
	}

	double sum_new_misguess = 0.0;
	for (int h_index = 0; h_index < num_instances; h_index++) {
		sum_new_misguess += (remaining_scores[h_index] - network_vals[h_index])
			* (remaining_scores[h_index] - network_vals[h_index]);
	}
	double new_average_misguess = sum_new_misguess / (double)num_instances;

	double sum_new_misguess_variance = 0.0;
	for (int h_index = 0; h_index < num_instances; h_index++) {
		double curr_misguess = (remaining_scores[h_index] - network_vals[h_index])
			* (remaining_scores[h_index] - network_vals[h_index]);
		sum_new_misguess_variance += (curr_misguess - new_average_misguess)
			* (curr_misguess - new_average_misguess);
	}
	double new_misguess_standard_deviation = sqrt(sum_new_misguess_variance / (double)num_instances);
	if (new_misguess_standard_deviation < MIN_STANDARD_DEVIATION) {
		new_misguess_standard_deviation = MIN_STANDARD_DEVIATION;
	}

	#if defined(MDEBUG) && MDEBUG
	if (should_add && rand()%2 == 0) {
	#else
	double new_improvement = average_misguess - new_average_misguess;
	double new_standard_deviation = min(misguess_standard_deviation, new_misguess_standard_deviation);
	double new_t_score = new_improvement / (new_standard_deviation / sqrt(num_instances));

	if (should_add && new_t_score > 2.326) {
	#endif /* MDEBUG */
		average_misguess = new_average_misguess;

		for (int i_index = (int)network_inputs.size()-1; i_index >= 0; i_index--) {
			vector<Input> remove_inputs = network_inputs;
			remove_inputs.erase(remove_inputs.begin() + i_index);

			Network* remove_network = new Network(new_network);
			remove_network->remove_input(i_index);

			vector<vector<double>> remove_input_vals = input_vals;
			vector<vector<bool>> remove_input_is_on = input_is_on;
			for (int d_index = 0; d_index < num_instances; d_index++) {
				remove_input_vals[d_index].erase(remove_input_vals[d_index].begin() + i_index);
				remove_input_is_on[d_index].erase(remove_input_is_on[d_index].begin() + i_index);
			}

			optimize_network(remove_input_vals,
							 remove_input_is_on,
							 remaining_scores,
							 remove_network);

			double remove_average_misguess;
			double remove_misguess_standard_deviation;
			measure_network(remove_input_vals,
							remove_input_is_on,
							remaining_scores,
							remove_network,
							remove_average_misguess,
							remove_misguess_standard_deviation);

			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			double remove_improvement = average_misguess - remove_average_misguess;
			double remove_standard_deviation = min(misguess_standard_deviation, remove_misguess_standard_deviation);
			double remove_t_score = remove_improvement / (remove_standard_deviation / sqrt(num_instances));

			if (remove_t_score > -0.674) {
			#endif /* MDEBUG */
				network_inputs = remove_inputs;

				delete new_network;
				new_network = remove_network;

				input_vals = remove_input_vals;
				input_is_on = remove_input_is_on;
			} else {
				delete remove_network;
			}
		}

		if (network_inputs.size() > 0) {
			Factor* new_factor = new Factor();
			new_factor->inputs = network_inputs;
			new_factor->network = new_network;
			new_factor->is_meaningful = true;

			scope->factors.push_back(new_factor);

			new_factor->link((int)scope->factors.size()-1);

			Input new_input;
			new_input.scope_context = {scope};
			new_input.factor_index = (int)scope->factors.size()-1;
			new_input.node_context = {-1};
			new_input.obs_index = -1;

			for (int h_index = 0; h_index < (int)scope->existing_scope_histories.size(); h_index++) {
				scope->existing_scope_histories[h_index]->factor_initialized.push_back(false);
				scope->existing_scope_histories[h_index]->factor_values.push_back(0.0);
			}

			factor_inputs.push_back(new_input);
			factor_input_averages.push_back(0.0);
			factor_input_standard_deviations.push_back(1.0);
			factor_weights.push_back(1.0);
		} else {
			delete new_network;
		}
	} else {
		delete new_network;
	}

	return true;
}
