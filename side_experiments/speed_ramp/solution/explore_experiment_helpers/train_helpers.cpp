/**
 * - is possible to solve XOR by selecting correlated
 *   - e.g.:
 *     - 0 0 -> 0
 *     - 0 1 -> 1
 *     - 1 0 -> 1
 *   - but gets increasingly difficult with XOR size
 *     - but solvable XOR size limited by number of inputs anyways
 *   - (with every dependency, XOR can be learned incrementally pattern by pattern)
 */

#include "explore_experiment.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#undef eigen_assert
#define eigen_assert(x) if (!(x)) {throw std::invalid_argument("Eigen error");}
#include <Eigen/Dense>

#include "branch_node.h"
#include "constants.h"
#include "factor.h"
#include "globals.h"
#include "helpers.h"
#include "network.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_TRIES = 2;
#else
const int TRAIN_TRIES = 10;
#endif /* MDEBUG */

const double SEED_RATIO = 0.3;

const int NUM_FACTORS = 4;
const int NUM_BEST_INPUTS = 5;
const int NUM_RANDOM_INPUTS = 5;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_ITERS = 30;
#else
const int TRAIN_ITERS = 300000;
#endif /* MDEBUG */

const double MIN_CONSIDER_HIT_PERCENT = 0.2;

const double UNIQUE_MAX_PCC = 0.7;

const double FACTOR_IMPACT_THRESHOLD = 0.1;

const int INPUT_NUM_HIGHEST = 4;
const int INPUT_NUM_RANDOM = 6;

class InputData {
public:
	std::vector<double> vals;
	std::vector<bool> is_on;

	double hit_percent;
	double average;
	double standard_deviation;
	double average_distance;

	std::vector<double> normalized_vals;
	double normalized_average;
	double normalized_standard_deviation;
};

void analyze_input(Input& input,
				   vector<ScopeHistory*>& scope_histories,
				   InputData* input_data) {
	vector<double> vals;
	int num_is_on = 0;
	int sum_num_distance = 0;
	for (int h_index = 0; h_index < (int)scope_histories.size(); h_index++) {
		int ending_num_actions = scope_histories[h_index]->num_actions_snapshot;

		double val;
		bool is_on;
		int num_actions_snapshot;
		fetch_input_helper(scope_histories[h_index],
						   input,
						   0,
						   val,
						   is_on,
						   num_actions_snapshot);

		input_data->vals.push_back(val);
		input_data->is_on.push_back(is_on);

		if (is_on) {
			vals.push_back(val);
			num_is_on++;
			sum_num_distance += (ending_num_actions - num_actions_snapshot);
		}
	}

	input_data->hit_percent = (double)num_is_on / (double)scope_histories.size();
	if (input_data->hit_percent >= MIN_CONSIDER_HIT_PERCENT) {
		double sum_vals = 0.0;
		for (int v_index = 0; v_index < (int)vals.size(); v_index++) {
			sum_vals += vals[v_index];
		}
		input_data->average = sum_vals / (double)vals.size();

		double sum_variance = 0.0;
		for (int v_index = 0; v_index < (int)vals.size(); v_index++) {
			sum_variance += (input_data->average - vals[v_index]) * (input_data->average - vals[v_index]);
		}
		input_data->standard_deviation = sqrt(sum_variance / (double)vals.size());

		if (input_data->standard_deviation >= MIN_STANDARD_DEVIATION) {
			input_data->average_distance = (double)sum_num_distance / (double)num_is_on;

			for (int h_index = 0; h_index < (int)input_data->vals.size(); h_index++) {
				if (input_data->is_on[h_index]) {
					double normalized_val = (input_data->vals[h_index] - input_data->average) / input_data->standard_deviation;
					input_data->normalized_vals.push_back(normalized_val);
				} else {
					input_data->normalized_vals.push_back(0.0);
				}
			}

			double sum_vals = 0.0;
			for (int h_index = 0; h_index < (int)input_data->normalized_vals.size(); h_index++) {
				sum_vals += input_data->normalized_vals[h_index];
			}
			input_data->normalized_average = sum_vals / (double)input_data->normalized_vals.size();

			double sum_variances = 0.0;
			for (int h_index = 0; h_index < (int)input_data->normalized_vals.size(); h_index++) {
				sum_variances += (input_data->normalized_vals[h_index] - input_data->normalized_average)
					* (input_data->normalized_vals[h_index] - input_data->normalized_average);
			}
			input_data->normalized_standard_deviation = sqrt(sum_variances / (double)input_data->normalized_vals.size());
		}
	}
}

void gather_inputs_helper(ScopeHistory* scope_history,
						  vector<Scope*>& scope_context,
						  vector<int>& node_context,
						  vector<ScopeHistory*>& scope_histories,
						  vector<pair<Input,InputData*>>& inputs_to_consider,
						  map<Input, InputData*>& input_tracker) {
	Scope* scope = scope_history->scope;

	uniform_int_distribution<int> inner_distribution(0, 2);
	uniform_real_distribution<double> distribution(0.0, 1.0);
	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		if (scope->nodes.find(it->first) != scope->nodes.end()) {
			AbstractNode* node = it->second->node;
			switch (node->type) {
			case NODE_TYPE_SCOPE:
				if (inner_distribution(generator) != 0) {
					ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;

					scope_context.push_back(scope);
					node_context.push_back(it->first);

					gather_inputs_helper(scope_node_history->scope_history,
										 scope_context,
										 node_context,
										 scope_histories,
										 inputs_to_consider,
										 input_tracker);

					scope_context.pop_back();
					node_context.pop_back();
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					scope_context.push_back(scope);
					node_context.push_back(it->first);

					Input input;
					input.scope_context = scope_context;
					input.factor_index = -1;
					input.node_context = node_context;
					input.obs_index = -1;

					map<Input, InputData*>::iterator it = input_tracker.find(input);
					if (it == input_tracker.end()) {
						InputData* input_data = new InputData();
						analyze_input(input,
									  scope_histories,
									  input_data);

						it = input_tracker.insert({input, input_data}).first;
					}

					if (it->second->hit_percent >= MIN_CONSIDER_HIT_PERCENT
							&& it->second->standard_deviation >= MIN_STANDARD_DEVIATION) {
						if (distribution(generator) <= 1.0 / (1.0 + sqrt(it->second->average_distance))) {
							inputs_to_consider.push_back({input, it->second});
						}
					}

					scope_context.pop_back();
					node_context.pop_back();
				}
				break;
			case NODE_TYPE_OBS:
				{
					ObsNodeHistory* obs_node_history = (ObsNodeHistory*)it->second;

					for (int o_index = 0; o_index < (int)obs_node_history->obs_history.size(); o_index++) {
						scope_context.push_back(scope);
						node_context.push_back(it->first);

						Input input;
						input.scope_context = scope_context;
						input.factor_index = -1;
						input.node_context = node_context;
						input.obs_index = o_index;

						map<Input, InputData*>::iterator it = input_tracker.find(input);
						if (it == input_tracker.end()) {
							InputData* input_data = new InputData();
							analyze_input(input,
										  scope_histories,
										  input_data);

							it = input_tracker.insert({input, input_data}).first;
						}

						if (it->second->hit_percent >= MIN_CONSIDER_HIT_PERCENT
								&& it->second->standard_deviation >= MIN_STANDARD_DEVIATION) {
							if (distribution(generator) <= 1.0 / (1.0 + sqrt(it->second->average_distance))) {
								inputs_to_consider.push_back({input, it->second});
							}
						}

						scope_context.pop_back();
						node_context.pop_back();
					}
				}
				break;
			}
		}
	}

	for (int f_index = 0; f_index < (int)scope->factors.size(); f_index++) {
		if (scope->factors[f_index]->is_meaningful) {
			scope_context.push_back(scope);
			node_context.push_back(-1);

			Input input;
			input.scope_context = scope_context;
			input.factor_index = f_index;
			input.node_context = node_context;
			input.obs_index = -1;

			map<Input, InputData*>::iterator it = input_tracker.find(input);
			if (it == input_tracker.end()) {
				InputData* input_data = new InputData();
				analyze_input(input,
							  scope_histories,
							  input_data);

				it = input_tracker.insert({input, input_data}).first;
			}

			if (it->second->hit_percent >= MIN_CONSIDER_HIT_PERCENT
					&& it->second->standard_deviation >= MIN_STANDARD_DEVIATION) {
				if (distribution(generator) <= 1.0 / (1.0 + sqrt(it->second->average_distance))) {
					inputs_to_consider.push_back({input, it->second});
				}
			}

			scope_context.pop_back();
			node_context.pop_back();
		}
	}
}

void try_helper(vector<ScopeHistory*>& scope_histories,
				vector<double>& target_val_histories,
				map<Input, InputData*>& input_tracker,
				double& constant,
				vector<Input>& factor_inputs,
				vector<vector<double>>& factor_vals,
				vector<double>& factor_input_averages,
				vector<double>& factor_input_standard_deviations,
				vector<double>& factor_weights,
				vector<Input>& network_inputs,
				vector<vector<double>>& network_vals,
				vector<vector<bool>>& network_is_on,
				Network*& network,
				double& average_misguess,
				double& seed_average_predicted_score,
				double& average_predicted_score,
				double& select_percentage) {
	Scope* scope = scope_histories[0]->scope;

	#if defined(MDEBUG) && MDEBUG
	int num_seeds = max(1.0, SEED_RATIO * (double)scope_histories.size());
	#else
	int num_seeds = SEED_RATIO * (double)scope_histories.size();
	#endif /* MDEBUG */

	vector<int> seed_indexes;
	vector<int> remaining_indexes(scope_histories.size());
	for (int i_index = 0; i_index < (int)scope_histories.size(); i_index++) {
		remaining_indexes[i_index] = i_index;
	}
	for (int s_index = 0; s_index < num_seeds; s_index++) {
		uniform_int_distribution<int> distribution(0, remaining_indexes.size()-1);
		int index = distribution(generator);
		seed_indexes.push_back(remaining_indexes[index]);
		remaining_indexes.erase(remaining_indexes.begin() + index);
	}

	double sum_seed_vals = 0.0;
	for (int s_index = 0; s_index < (int)seed_indexes.size(); s_index++) {
		sum_seed_vals += target_val_histories[seed_indexes[s_index]];
	}
	double seed_val_average = sum_seed_vals / (double)seed_indexes.size();

	double sum_seed_variances = 0.0;
	for (int s_index = 0; s_index < (int)seed_indexes.size(); s_index++) {
		sum_seed_variances += (target_val_histories[seed_indexes[s_index]] - seed_val_average)
			* (target_val_histories[seed_indexes[s_index]] - seed_val_average);
	}
	double seed_val_standard_deviation = sqrt(sum_seed_variances / (double)seed_indexes.size());

	bool train_factor_success = false;

	vector<InputData*> factor_input_datas;
	if (seed_val_standard_deviation >= MIN_STANDARD_DEVIATION) {
		vector<double> normalized_seed_target_vals(seed_indexes.size());
		for (int s_index = 0; s_index < (int)seed_indexes.size(); s_index++) {
			normalized_seed_target_vals[s_index] = target_val_histories[seed_indexes[s_index]] - seed_val_average;
		}

		vector<pair<double, pair<Input,InputData*>>> factor_pccs;
		for (int f_index = 0; f_index < (int)scope->factors.size(); f_index++) {
			Input input;
			input.scope_context = {scope};
			input.factor_index = f_index;
			input.node_context = {-1};
			input.obs_index = -1;

			map<Input, InputData*>::iterator it = input_tracker.find(input);
			if (it == input_tracker.end()) {
				InputData* input_data = new InputData();
				analyze_input(input,
							  scope_histories,
							  input_data);

				it = input_tracker.insert({input, input_data}).first;
			}

			if (it->second->hit_percent >= MIN_CONSIDER_HIT_PERCENT
					&& it->second->standard_deviation >= MIN_STANDARD_DEVIATION) {
				double sum_vals = 0.0;
				for (int s_index = 0; s_index < (int)seed_indexes.size(); s_index++) {
					sum_vals += it->second->normalized_vals[seed_indexes[s_index]];
				}
				double val_average = sum_vals / (double)seed_indexes.size();

				double sum_variances = 0.0;
				for (int s_index = 0; s_index < (int)seed_indexes.size(); s_index++) {
					sum_variances += (it->second->normalized_vals[seed_indexes[s_index]] - val_average)
						* (it->second->normalized_vals[seed_indexes[s_index]] - val_average);
				}
				double val_standard_deviation = sqrt(sum_variances / (double)seed_indexes.size());

				if (val_standard_deviation >= MIN_STANDARD_DEVIATION) {
					double sum_covariance = 0.0;
					for (int s_index = 0; s_index < (int)seed_indexes.size(); s_index++) {
						sum_covariance += normalized_seed_target_vals[s_index]
							* (it->second->normalized_vals[seed_indexes[s_index]] - val_average);
					}
					double covariance = sum_covariance / (double)seed_indexes.size();

					double pcc = covariance / seed_val_standard_deviation / val_standard_deviation;

					factor_pccs.push_back({abs(pcc), {input, it->second}});
				}
			}
		}
		sort(factor_pccs.begin(), factor_pccs.end());

		vector<Input> new_factor_inputs;
		vector<InputData*> new_factor_input_datas;
		for (int f_index = (int)factor_pccs.size()-1; f_index >= 0; f_index--) {
			InputData* potential_input_data = factor_pccs[f_index].second.second;

			bool should_add = true;
			for (int e_index = 0; e_index < (int)new_factor_inputs.size(); e_index++) {
				InputData* existing_input_data = new_factor_input_datas[e_index];

				double sum_covariance = 0.0;
				for (int h_index = 0; h_index < (int)potential_input_data->normalized_vals.size(); h_index++) {
					sum_covariance += (potential_input_data->normalized_vals[h_index] - potential_input_data->normalized_average)
						* (existing_input_data->normalized_vals[h_index] - existing_input_data->normalized_average);
				}
				double covariance = sum_covariance / (double)potential_input_data->normalized_vals.size();

				double pcc = covariance / potential_input_data->normalized_standard_deviation / existing_input_data->normalized_standard_deviation;

				if (abs(pcc) > UNIQUE_MAX_PCC) {
					should_add = false;
					break;
				}
			}

			if (should_add) {
				new_factor_inputs.push_back(factor_pccs[f_index].second.first);
				new_factor_input_datas.push_back(factor_pccs[f_index].second.second);
			}

			factor_pccs.pop_back();

			if (new_factor_inputs.size() >= NUM_FACTORS) {
				break;
			}
		}

		Eigen::MatrixXd inputs(remaining_indexes.size(), 1 + new_factor_inputs.size());
		uniform_real_distribution<double> noise_distribution(-0.001, 0.001);
		/**
		 * - add some noise to prevent extremes
		 */
		for (int i_index = 0; i_index < (int)remaining_indexes.size(); i_index++) {
			inputs(i_index, 0) = 1.0;
			for (int f_index = 0; f_index < (int)new_factor_inputs.size(); f_index++) {
				inputs(i_index, 1 + f_index) = new_factor_input_datas[f_index]
					->normalized_vals[remaining_indexes[i_index]]
						+ noise_distribution(generator);
			}
		}

		Eigen::VectorXd outputs(remaining_indexes.size());
		for (int i_index = 0; i_index < (int)remaining_indexes.size(); i_index++) {
			outputs(i_index) = target_val_histories[remaining_indexes[i_index]];
		}

		train_factor_success = true;
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
			for (int f_index = 0; f_index < (int)factor_inputs.size(); f_index++) {
				if (abs(weights(1 + f_index)) > REGRESSION_WEIGHT_LIMIT) {
					cout << "abs(weights(1 + f_index)): " << abs(weights(1 + f_index)) << endl;
					train_factor_success = false;
				}
			}
		}
		#endif /* MDEBUG */

		if (train_factor_success) {
			vector<double> seed_predicted_vals(seed_indexes.size());
			for (int s_index = 0; s_index < (int)seed_indexes.size(); s_index++) {
				double sum_score = weights(0);
				for (int f_index = 0; f_index < (int)new_factor_inputs.size(); f_index++) {
					sum_score += weights(1 + f_index) * new_factor_input_datas[f_index]->normalized_vals[seed_indexes[s_index]];
				}
				seed_predicted_vals[s_index] = sum_score;
			}

			double sum_seed_factor_misguess = 0.0;
			for (int s_index = 0; s_index < (int)seed_indexes.size(); s_index++) {
				sum_seed_factor_misguess += (target_val_histories[seed_indexes[s_index]] - seed_predicted_vals[s_index])
					* (target_val_histories[seed_indexes[s_index]] - seed_predicted_vals[s_index]);
			}

			#if defined(MDEBUG) && MDEBUG
			if (sum_seed_factor_misguess >= sum_seed_variances && rand()%2 == 0) {
			#else
			if (sum_seed_factor_misguess >= sum_seed_variances) {
			#endif /* MDEBUG */
				train_factor_success = false;
			}
		}

		if (train_factor_success) {
			factor_inputs = new_factor_inputs;
			for (int f_index = 0; f_index < (int)new_factor_inputs.size(); f_index++) {
				factor_input_averages.push_back(new_factor_input_datas[f_index]->average);
				factor_input_standard_deviations.push_back(new_factor_input_datas[f_index]->standard_deviation);
			}
			factor_input_datas = new_factor_input_datas;

			constant = weights(0);
			for (int f_index = 0; f_index < (int)new_factor_inputs.size(); f_index++) {
				factor_weights.push_back(weights(1 + f_index));
			}
		}
	}

	if (!train_factor_success) {
		double sum_remaining_scores = 0.0;
		for (int i_index = 0; i_index < (int)remaining_indexes.size(); i_index++) {
			sum_remaining_scores += target_val_histories[remaining_indexes[i_index]];
		}
		constant = sum_remaining_scores / (double)remaining_indexes.size();
	}

	for (int h_index = 0; h_index < (int)scope_histories.size(); h_index++) {
		vector<double> curr_factor_vals(factor_inputs.size());
		for (int f_index = 0; f_index < (int)factor_inputs.size(); f_index++) {
			curr_factor_vals[f_index] = factor_input_datas[f_index]->normalized_vals[h_index];
		}
		factor_vals.push_back(curr_factor_vals);
	}

	vector<double> sum_vals(scope_histories.size());
	vector<double> remaining_scores(scope_histories.size());
	for (int h_index = 0; h_index < (int)scope_histories.size(); h_index++) {
		double sum_score = constant;
		for (int f_index = 0; f_index < (int)factor_inputs.size(); f_index++) {
			sum_score += factor_weights[f_index]
				* factor_input_datas[f_index]->normalized_vals[h_index];
		}

		sum_vals[h_index] = sum_score;
		remaining_scores[h_index] = target_val_histories[h_index] - sum_score;
	}

	/**
	 * - simply gather inputs to consider from a single run
	 */
	vector<Scope*> scope_context;
	vector<int> node_context;
	vector<pair<Input, InputData*>> inputs_to_consider;
	gather_inputs_helper(scope_histories[seed_indexes[0]],
						 scope_context,
						 node_context,
						 scope_histories,
						 inputs_to_consider,
						 input_tracker);

	double sum_seed_remaining_vals = 0.0;
	for (int s_index = 0; s_index < (int)seed_indexes.size(); s_index++) {
		sum_seed_remaining_vals += remaining_scores[seed_indexes[s_index]];
	}
	double seed_remaining_val_average = sum_seed_remaining_vals / (double)seed_indexes.size();

	double sum_seed_remaining_variances = 0.0;
	for (int s_index = 0; s_index < (int)seed_indexes.size(); s_index++) {
		sum_seed_remaining_variances += (remaining_scores[seed_indexes[s_index]] - seed_remaining_val_average)
			* (remaining_scores[seed_indexes[s_index]] - seed_remaining_val_average);
	}
	double seed_remaining_val_standard_deviation = sqrt(sum_seed_remaining_variances / (double)seed_indexes.size());

	if (inputs_to_consider.size() > 0
			&& seed_remaining_val_standard_deviation >= MIN_STANDARD_DEVIATION) {
		vector<double> normalized_seed_remaining_target_vals(seed_indexes.size());
		for (int s_index = 0; s_index < (int)seed_indexes.size(); s_index++) {
			normalized_seed_remaining_target_vals[s_index] = remaining_scores[seed_indexes[s_index]] - seed_remaining_val_average;
		}

		vector<pair<double, pair<Input,InputData*>>> input_pccs;
		for (int i_index = 0; i_index < (int)inputs_to_consider.size(); i_index++) {
			if (inputs_to_consider[i_index].second->hit_percent >= MIN_CONSIDER_HIT_PERCENT
					&& inputs_to_consider[i_index].second->standard_deviation >= MIN_STANDARD_DEVIATION) {
				double sum_vals = 0.0;
				for (int s_index = 0; s_index < (int)seed_indexes.size(); s_index++) {
					sum_vals += inputs_to_consider[i_index].second->normalized_vals[seed_indexes[s_index]];
				}
				double val_average = sum_vals / (double)seed_indexes.size();

				double sum_variances = 0.0;
				for (int s_index = 0; s_index < (int)seed_indexes.size(); s_index++) {
					sum_variances += (inputs_to_consider[i_index].second->normalized_vals[seed_indexes[s_index]] - val_average)
						* (inputs_to_consider[i_index].second->normalized_vals[seed_indexes[s_index]] - val_average);
				}
				double val_standard_deviation = sqrt(sum_variances / (double)seed_indexes.size());

				if (val_standard_deviation >= MIN_STANDARD_DEVIATION) {
					double sum_covariance = 0.0;
					for (int s_index = 0; s_index < (int)seed_indexes.size(); s_index++) {
						sum_covariance += normalized_seed_remaining_target_vals[s_index]
							* (inputs_to_consider[i_index].second->normalized_vals[seed_indexes[s_index]] - val_average);
					}
					double covariance = sum_covariance / (double)seed_indexes.size();

					double pcc = covariance / seed_remaining_val_standard_deviation / val_standard_deviation;

					input_pccs.push_back({abs(pcc), {inputs_to_consider[i_index].first, inputs_to_consider[i_index].second}});
				}
			}
		}
		sort(input_pccs.begin(), input_pccs.end());

		vector<Input> new_network_inputs;
		vector<InputData*> new_network_input_datas;
		vector<double> input_averages;
		vector<double> input_standard_deviations;
		for (int i_index = (int)input_pccs.size()-1; i_index >= 0; i_index--) {
			InputData* potential_input_data = input_pccs[i_index].second.second;

			bool should_add = true;
			for (int e_index = 0; e_index < (int)new_network_inputs.size(); e_index++) {
				InputData* existing_input_data = new_network_input_datas[e_index];

				double sum_covariance = 0.0;
				for (int h_index = 0; h_index < (int)potential_input_data->normalized_vals.size(); h_index++) {
					sum_covariance += (potential_input_data->normalized_vals[h_index] - potential_input_data->normalized_average)
						* (existing_input_data->normalized_vals[h_index] - existing_input_data->normalized_average);
				}
				double covariance = sum_covariance / (double)potential_input_data->normalized_vals.size();

				double pcc = covariance / potential_input_data->normalized_standard_deviation / existing_input_data->normalized_standard_deviation;

				if (abs(pcc) > UNIQUE_MAX_PCC) {
					should_add = false;
					break;
				}
			}

			if (should_add) {
				new_network_inputs.push_back(input_pccs[i_index].second.first);
				input_averages.push_back(potential_input_data->average);
				input_standard_deviations.push_back(potential_input_data->standard_deviation);

				new_network_input_datas.push_back(input_pccs[i_index].second.second);
			}

			input_pccs.pop_back();

			if (new_network_inputs.size() >= NUM_BEST_INPUTS) {
				break;
			}
		}
		while (input_pccs.size() > 0) {
			uniform_int_distribution<int> random_distribution(0, input_pccs.size()-1);
			int random_index = random_distribution(generator);

			InputData* potential_input_data = input_pccs[random_index].second.second;

			bool should_add = true;
			for (int e_index = 0; e_index < (int)new_network_inputs.size(); e_index++) {
				InputData* existing_input_data = new_network_input_datas[e_index];

				double sum_covariance = 0.0;
				for (int h_index = 0; h_index < (int)potential_input_data->normalized_vals.size(); h_index++) {
					sum_covariance += (potential_input_data->normalized_vals[h_index] - potential_input_data->normalized_average)
						* (existing_input_data->normalized_vals[h_index] - existing_input_data->normalized_average);
				}
				double covariance = sum_covariance / (double)potential_input_data->normalized_vals.size();

				double pcc = covariance / potential_input_data->normalized_standard_deviation / existing_input_data->normalized_standard_deviation;

				if (abs(pcc) > UNIQUE_MAX_PCC) {
					should_add = false;
					break;
				}
			}

			if (should_add) {
				new_network_inputs.push_back(input_pccs[random_index].second.first);
				input_averages.push_back(potential_input_data->average);
				input_standard_deviations.push_back(potential_input_data->standard_deviation);

				new_network_input_datas.push_back(input_pccs[random_index].second.second);
			}

			input_pccs.erase(input_pccs.begin() + random_index);

			if (new_network_inputs.size() >= NUM_BEST_INPUTS + NUM_RANDOM_INPUTS) {
				break;
			}
		}

		for (int h_index = 0; h_index < (int)scope_histories.size(); h_index++) {
			vector<double> curr_input_vals(new_network_inputs.size());
			vector<bool> curr_input_is_on(new_network_inputs.size());
			for (int i_index = 0; i_index < (int)new_network_inputs.size(); i_index++) {
				curr_input_vals[i_index] = new_network_input_datas[i_index]->vals[h_index];
				curr_input_is_on[i_index] = new_network_input_datas[i_index]->is_on[h_index];
			}
			network_vals.push_back(curr_input_vals);
			network_is_on.push_back(curr_input_is_on);
		}

		Network* new_network = new Network((int)new_network_inputs.size(),
										   input_averages,
										   input_standard_deviations);

		uniform_int_distribution<int> input_distribution(0, remaining_indexes.size()-1);
		uniform_int_distribution<int> drop_distribution(0, 9);
		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
			int rand_index = remaining_indexes[input_distribution(generator)];

			vector<bool> w_drop(new_network_inputs.size());
			for (int i_index = 0; i_index < (int)new_network_inputs.size(); i_index++) {
				if (drop_distribution(generator) == 0) {
					w_drop[i_index] = false;
				} else {
					w_drop[i_index] = network_is_on[rand_index][i_index];
				}
			}

			new_network->activate(network_vals[rand_index],
								  w_drop);

			double error = remaining_scores[rand_index] - new_network->output->acti_vals[0];

			new_network->backprop(error);
		}

		vector<double> network_outputs(scope_histories.size());
		for (int h_index = 0; h_index < (int)scope_histories.size(); h_index++) {
			new_network->activate(network_vals[h_index],
								  network_is_on[h_index]);

			network_outputs[h_index] = new_network->output->acti_vals[0];
		}

		double sum_default_misguess = 0.0;
		for (int s_index = 0; s_index < (int)seed_indexes.size(); s_index++) {
			sum_default_misguess += remaining_scores[seed_indexes[s_index]] * remaining_scores[seed_indexes[s_index]];
		}

		double sum_network_misguess = 0.0;
		for (int s_index = 0; s_index < (int)seed_indexes.size(); s_index++) {
			sum_network_misguess += (remaining_scores[seed_indexes[s_index]] - network_outputs[seed_indexes[s_index]])
				* (remaining_scores[seed_indexes[s_index]] - network_outputs[seed_indexes[s_index]]);
		}

		#if defined(MDEBUG) && MDEBUG
		if (sum_network_misguess < sum_default_misguess || rand()%4 != 0) {
		#else
		if (sum_network_misguess < sum_default_misguess) {
		#endif /* MDEBUG */
			for (int h_index = 0; h_index < (int)scope_histories.size(); h_index++) {
				sum_vals[h_index] += network_outputs[h_index];
			}

			network_inputs = new_network_inputs;
			network = new_network;
		} else {
			delete new_network;
		}
	}

	/**
	 * - verification
	 */
	// for (int h_index = 0; h_index < (int)scope_histories.size(); h_index++) {
	// 	double curr_sum_vals = constant;

	// 	for (int i_index = 0; i_index < (int)factor_inputs.size(); i_index++) {
	// 		double val;
	// 		bool is_on;
	// 		fetch_input_helper(scope_histories[h_index],
	// 						   factor_inputs[i_index],
	// 						   0,
	// 						   val,
	// 						   is_on);
	// 		if (is_on) {
	// 			double normalized_val = (val - factor_input_averages[i_index]) / factor_input_standard_deviations[i_index];
	// 			curr_sum_vals += factor_weights[i_index] * normalized_val;
	// 		}
	// 	}

	// 	if (network != NULL) {
	// 		vector<double> input_vals(network_inputs.size());
	// 		vector<bool> input_is_on(network_inputs.size());
	// 		for (int i_index = 0; i_index < (int)network_inputs.size(); i_index++) {
	// 			double val;
	// 			bool is_on;
	// 			fetch_input_helper(scope_histories[h_index],
	// 							   network_inputs[i_index],
	// 							   0,
	// 							   val,
	// 							   is_on);
	// 			input_vals[i_index] = val;
	// 			input_is_on[i_index] = is_on;
	// 		}
	// 		network->activate(input_vals,
	// 						  input_is_on);
	// 		curr_sum_vals += network->output->acti_vals[0];
	// 	}

	// 	if (curr_sum_vals != sum_vals[h_index]) {
	// 		throw invalid_argument("curr_sum_vals != sum_vals[h_index]");
	// 	}
	// }

	double seed_sum_predicted_score = 0.0;
	for (int s_index = 0; s_index < (int)seed_indexes.size(); s_index++) {
		if (sum_vals[seed_indexes[s_index]] >= 0.0) {
			seed_sum_predicted_score += target_val_histories[seed_indexes[s_index]];
		}
	}
	seed_average_predicted_score = seed_sum_predicted_score / (double)seed_indexes.size();

	/**
	 * - check overall predicted in addition to seed
	 *   - in case difficult samples not in seed
	 */
	double sum_predicted_score = 0.0;
	for (int h_index = 0; h_index < (int)scope_histories.size(); h_index++) {
		if (sum_vals[h_index] >= 0.0) {
			sum_predicted_score += target_val_histories[h_index];
		}
	}
	average_predicted_score = sum_predicted_score / (double)scope_histories.size();

	double sum_misguess = 0.0;
	for (int h_index = 0; h_index < (int)scope_histories.size(); h_index++) {
		sum_misguess += (target_val_histories[h_index] - sum_vals[h_index])
			* (target_val_histories[h_index] - sum_vals[h_index]);
	}
	average_misguess = sum_misguess / (double)scope_histories.size();

	#if defined(MDEBUG) && MDEBUG
	if (rand()%2 == 0) {
		select_percentage = 0.5;
	} else {
		select_percentage = 0.0;
	}
	#else
	int num_positive = 0;
	for (int i_index = 0; i_index < (int)scope_histories.size(); i_index++) {
		if (sum_vals[i_index] >= 0.0) {
			num_positive++;
		}
	}
	select_percentage = (double)num_positive / (double)scope_histories.size();
	#endif /* MDEBUG */
}

bool train_existing_helper(AbstractNode* node_context,
						   bool is_branch,
						   SolutionWrapper* wrapper,
						   double& constant,
						   vector<Input>& inputs,
						   vector<double>& input_averages,
						   vector<double>& input_standard_deviations,
						   vector<double>& weights,
						   vector<Input>& network_inputs,
						   Network*& network) {
	vector<ScopeHistory*> scope_histories;
	vector<double> target_val_histories;
	int num_hits = 0;
	double average_score = wrapper->solution->sum_scores / (double)HISTORIES_NUM_SAVE;
	for (list<pair<ScopeHistory*,double>>::iterator it = wrapper->solution->existing_scope_histories.begin();
			it != wrapper->solution->existing_scope_histories.end(); it++) {
		int start_size = (int)scope_histories.size();

		vector<ScopeHistory*> scope_context_histories;
		double true_diff = it->second - average_score;
		fetch_histories_helper(it->first,
							   scope_context_histories,
							   true_diff,
							   node_context,
							   is_branch,
							   scope_histories,
							   target_val_histories);

		if ((int)scope_histories.size() > start_size) {
			num_hits++;
		}
	}

	if (num_hits >= EXPERIMENT_MIN_HIT) {
		map<Input, InputData*> input_tracker;

		double best_average_misguess = numeric_limits<double>::max();
		for (int t_index = 0; t_index < TRAIN_TRIES; t_index++) {
			double curr_constant;
			vector<Input> curr_factor_inputs;
			vector<vector<double>> curr_factor_vals;
			vector<double> curr_factor_input_averages;
			vector<double> curr_factor_input_standard_deviations;
			vector<double> curr_factor_weights;
			vector<Input> curr_network_inputs;
			vector<vector<double>> curr_network_vals;
			vector<vector<bool>> curr_network_is_on;
			Network* curr_network = NULL;
			double curr_average_misguess;
			double curr_seed_average_predicted_score;
			double curr_average_predicted_score;
			double curr_select_percentage;
			try_helper(scope_histories,
					   target_val_histories,
					   input_tracker,
					   curr_constant,
					   curr_factor_inputs,
					   curr_factor_vals,
					   curr_factor_input_averages,
					   curr_factor_input_standard_deviations,
					   curr_factor_weights,
					   curr_network_inputs,
					   curr_network_vals,
					   curr_network_is_on,
					   curr_network,
					   curr_average_misguess,
					   curr_seed_average_predicted_score,
					   curr_average_predicted_score,
					   curr_select_percentage);

			#if defined(MDEBUG) && MDEBUG
			if (curr_average_misguess < best_average_misguess || rand()%2 == 0) {
			#else
			if (curr_average_misguess < best_average_misguess) {
			#endif /* MDEBUG */
				best_average_misguess = curr_average_misguess;

				constant = curr_constant;
				inputs = curr_factor_inputs;
				input_averages = curr_factor_input_averages;
				input_standard_deviations = curr_factor_input_standard_deviations;
				weights = curr_factor_weights;
				network_inputs = curr_network_inputs;
				if (network != NULL) {
					delete network;
				}
				network = curr_network;
				curr_network = NULL;
			}

			if (curr_network != NULL) {
				delete curr_network;
			}
		}

		for (map<Input, InputData*>::iterator it = input_tracker.begin();
				it != input_tracker.end(); it++) {
			delete it->second;
		}

		return true;
	} else {
		return false;
	}
}

bool ExploreExperiment::train_new_helper() {
	map<Input, InputData*> input_tracker;

	double best_average_misguess = numeric_limits<double>::max();

	double best_constant;
	vector<Input> best_factor_inputs;
	vector<vector<double>> best_factor_vals;
	vector<double> best_factor_input_averages;
	vector<double> best_factor_input_standard_deviations;
	vector<double> best_factor_weights;
	vector<Input> best_network_inputs;
	vector<vector<double>> best_network_vals;
	vector<vector<bool>> best_network_is_on;
	Network* best_network = NULL;
	double best_seed_average_predicted_score = 0.0;
	double best_average_predicted_score = 0.0;
	double best_select_percentage = 0.0;

	for (int t_index = 0; t_index < TRAIN_TRIES; t_index++) {
		double curr_constant;
		vector<Input> curr_factor_inputs;
		vector<vector<double>> curr_factor_vals;
		vector<double> curr_factor_input_averages;
		vector<double> curr_factor_input_standard_deviations;
		vector<double> curr_factor_weights;
		vector<Input> curr_network_inputs;
		vector<vector<double>> curr_network_vals;
		vector<vector<bool>> curr_network_is_on;
		Network* curr_network = NULL;
		double curr_average_misguess;
		double curr_seed_average_predicted_score;
		double curr_average_predicted_score;
		double curr_select_percentage;
		try_helper(this->scope_histories,
				   this->target_val_histories,
				   input_tracker,
				   curr_constant,
				   curr_factor_inputs,
				   curr_factor_vals,
				   curr_factor_input_averages,
				   curr_factor_input_standard_deviations,
				   curr_factor_weights,
				   curr_network_inputs,
				   curr_network_vals,
				   curr_network_is_on,
				   curr_network,
				   curr_average_misguess,
				   curr_seed_average_predicted_score,
				   curr_average_predicted_score,
				   curr_select_percentage);

		// // temp
		// cout << t_index << endl;
		// cout << "curr_average_misguess: " << curr_average_misguess << endl;
		// cout << "curr_seed_average_predicted_score: " << curr_seed_average_predicted_score << endl;
		// cout << "curr_average_predicted_score: " << curr_average_predicted_score << endl;
		// cout << "curr_select_percentage: " << curr_select_percentage << endl;

		#if defined(MDEBUG) && MDEBUG
		if (curr_average_misguess < best_average_misguess || rand()%2 == 0) {
		#else
		if (curr_average_misguess < best_average_misguess) {
		#endif /* MDEBUG */
			// // temp
			// cout << "update" << endl;

			best_average_misguess = curr_average_misguess;

			best_constant = curr_constant;
			best_factor_inputs = curr_factor_inputs;
			best_factor_vals = curr_factor_vals;
			best_factor_input_averages = curr_factor_input_averages;
			best_factor_input_standard_deviations = curr_factor_input_standard_deviations;
			best_factor_weights = curr_factor_weights;
			best_network_inputs = curr_network_inputs;
			best_network_vals = curr_network_vals;
			best_network_is_on = curr_network_is_on;
			if (best_network != NULL) {
				delete best_network;
			}
			best_network = curr_network;
			curr_network = NULL;
			best_seed_average_predicted_score = curr_seed_average_predicted_score;
			best_average_predicted_score = curr_average_predicted_score;
			best_select_percentage = curr_select_percentage;
		}

		if (curr_network != NULL) {
			delete curr_network;
		}
	}

	for (map<Input, InputData*>::iterator it = input_tracker.begin();
			it != input_tracker.end(); it++) {
		delete it->second;
	}

	#if defined(MDEBUG) && MDEBUG
	if ((best_seed_average_predicted_score >= 0.0
				&& best_average_predicted_score >= 0.0
				&& best_select_percentage > 0) || rand()%2 == 0) {
	#else
	if (best_seed_average_predicted_score >= 0.0
				&& best_average_predicted_score >= 0.0
				&& best_select_percentage > 0) {
	#endif /* MDEBUG */
		this->new_constant = best_constant;
		this->new_inputs = best_factor_inputs;
		this->factor_vals = best_factor_vals;
		this->new_input_averages = best_factor_input_averages;
		this->new_input_standard_deviations = best_factor_input_standard_deviations;
		this->new_weights = best_factor_weights;
		this->new_network_inputs = best_network_inputs;
		this->network_vals = best_network_vals;
		this->network_is_on = best_network_is_on;
		if (this->new_network != NULL) {
			delete this->new_network;
		}
		this->new_network = best_network;
		best_network = NULL;
		this->select_percentage = best_select_percentage;

		return true;
	} else {
		if (best_network != NULL) {
			delete best_network;
		}

		return false;
	}
}
