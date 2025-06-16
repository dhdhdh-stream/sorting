#include "solution_helpers.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#undef eigen_assert
#define eigen_assert(x) if (!(x)) {throw std::invalid_argument("Eigen error");}
#include <Eigen/Dense>

#include "abstract_experiment.h"
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

const double MIN_CONSIDER_HIT_PERCENT = 0.2;

const int FACTOR_NUM_GATHER = 5;
const int FACTOR_NUM_PER_GATHER = 5;

/**
 * - when there's correlation, weights can get strange values(?)
 */
const double REGRESSION_WEIGHT_LIMIT = 100000.0;
const double REGRESSION_FAIL_MULTIPLIER = 1000.0;

const double FACTOR_IMPACT_THRESHOLD = 0.1;

const int INPUT_NUM_HIGHEST = 2;
const int INPUT_NUM_RANDOM = 3;

class InputData {
public:
	double hit_percent;
	double average;
	double standard_deviation;
};

void factor_best_helper(vector<double>& target_val_histories,
						vector<int>& best_indexes) {
	best_indexes = vector<int>(FACTOR_NUM_GATHER, -1);
	for (int h_index = 0; h_index < (int)target_val_histories.size(); h_index++) {
		if (best_indexes.back() == -1
				|| target_val_histories[h_index] > target_val_histories[best_indexes.back()]) {
			best_indexes.back() = h_index;

			int curr_index = FACTOR_NUM_GATHER-2;
			while (true) {
				if (curr_index < 0) {
					break;
				}

				if (best_indexes[curr_index] == -1
						|| target_val_histories[best_indexes[curr_index + 1]] > target_val_histories[best_indexes[curr_index]]) {
					double temp = best_indexes[curr_index + 1];
					best_indexes[curr_index + 1] = best_indexes[curr_index];
					best_indexes[curr_index] = temp;

					curr_index--;
				} else {
					break;
				}
			}
		}
	}
}

void factor_worst_helper(vector<double>& target_val_histories,
						 vector<int>& worst_indexes) {
	worst_indexes = vector<int>(FACTOR_NUM_GATHER, -1);
	for (int h_index = 0; h_index < (int)target_val_histories.size(); h_index++) {
		if (worst_indexes.back() == -1
				|| target_val_histories[h_index] < target_val_histories[worst_indexes.back()]) {
			worst_indexes.back() = h_index;

			int curr_index = FACTOR_NUM_GATHER-2;
			while (true) {
				if (curr_index < 0) {
					break;
				}

				if (worst_indexes[curr_index] == -1
						|| target_val_histories[worst_indexes[curr_index + 1]] < target_val_histories[worst_indexes[curr_index]]) {
					double temp = worst_indexes[curr_index + 1];
					worst_indexes[curr_index + 1] = worst_indexes[curr_index];
					worst_indexes[curr_index] = temp;

					curr_index--;
				} else {
					break;
				}
			}
		}
	}
}

void analyze_input(Input& input,
					vector<ScopeHistory*>& scope_histories,
					InputData& input_data) {
	vector<double> vals;
	int num_is_on = 0;
	for (int h_index = 0; h_index < (int)scope_histories.size(); h_index++) {
		double val;
		bool is_on;
		fetch_input_helper(scope_histories[h_index],
						   input,
						   0,
						   val,
						   is_on);
		if (is_on) {
			vals.push_back(val);
			num_is_on++;
		}
	}

	input_data.hit_percent = (double)num_is_on / (double)scope_histories.size();
	if (input_data.hit_percent >= MIN_CONSIDER_HIT_PERCENT) {
		double sum_vals = 0.0;
		for (int v_index = 0; v_index < (int)vals.size(); v_index++) {
			sum_vals += vals[v_index];
		}
		input_data.average = sum_vals / (double)vals.size();

		double sum_variance = 0.0;
		for (int v_index = 0; v_index < (int)vals.size(); v_index++) {
			sum_variance += (input_data.average - vals[v_index]) * (input_data.average - vals[v_index]);
		}
		input_data.standard_deviation = sqrt(sum_variance / (double)vals.size());
	}
}

void highest_t_factor_helper(ScopeHistory* scope_history,
							 vector<Scope*>& scope_context,
							 vector<int>& node_context,
							 vector<Input>& highest_t,
							 vector<double>& highest_t_scores,
							 vector<ScopeHistory*>& scope_histories,
							 map<Input, InputData>& input_tracker) {
	Scope* scope = scope_history->scope;

	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		AbstractNode* node = it->second->node;
		switch (node->type) {
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;

				scope_context.push_back(scope);
				node_context.push_back(it->first);

				highest_t_factor_helper(scope_node_history->scope_history,
										scope_context,
										node_context,
										highest_t,
										highest_t_scores,
										scope_histories,
										input_tracker);

				scope_context.pop_back();
				node_context.pop_back();
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)node;
				ObsNodeHistory* obs_node_history = (ObsNodeHistory*)it->second;
				for (int f_index = 0; f_index < (int)obs_node->factors.size(); f_index++) {
					scope_context.push_back(scope);
					node_context.push_back(it->first);

					Input input;
					input.scope_context = scope_context;
					input.node_context = node_context;
					input.factor_index = f_index;
					input.obs_index = -1;

					map<Input, InputData>::iterator it = input_tracker.find(input);
					if (it == input_tracker.end()) {
						InputData input_data;
						analyze_input(input,
									  scope_histories,
									  input_data);

						it = input_tracker.insert({input, input_data}).first;
					}

					if (it->second.hit_percent >= MIN_CONSIDER_HIT_PERCENT
							&& it->second.standard_deviation > 0.0) {
						double curr_val = obs_node_history->factor_values[f_index];
						double curr_t_score = abs(curr_val - it->second.average) / it->second.standard_deviation;
						if (curr_t_score > highest_t_scores.back()) {
							highest_t.back() = input;
							highest_t_scores.back() = curr_t_score;

							int curr_index = FACTOR_NUM_PER_GATHER-2;
							while (true) {
								if (curr_index < 0) {
									break;
								}

								if (highest_t_scores[curr_index + 1] > highest_t_scores[curr_index]) {
									Input temp = highest_t[curr_index + 1];
									double temp_score = highest_t_scores[curr_index + 1];
									highest_t[curr_index + 1] = highest_t[curr_index];
									highest_t_scores[curr_index + 1] = highest_t_scores[curr_index];
									highest_t[curr_index] = temp;
									highest_t_scores[curr_index] = temp_score;

									curr_index--;
								} else {
									break;
								}
							}
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

void highest_t_input_helper(ScopeHistory* scope_history,
							vector<Scope*>& scope_context,
							vector<int>& node_context,
							vector<Input>& highest_t,
							vector<double>& highest_t_scores,
							vector<ScopeHistory*>& scope_histories,
							map<Input, InputData>& input_tracker) {
	Scope* scope = scope_history->scope;

	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		AbstractNode* node = it->second->node;
		switch (node->type) {
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;

				scope_context.push_back(scope);
				node_context.push_back(it->first);

				highest_t_input_helper(scope_node_history->scope_history,
									   scope_context,
									   node_context,
									   highest_t,
									   highest_t_scores,
									   scope_histories,
									   input_tracker);

				scope_context.pop_back();
				node_context.pop_back();
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;

				scope_context.push_back(scope);
				node_context.push_back(it->first);

				Input input;
				input.scope_context = scope_context;
				input.node_context = node_context;
				input.factor_index = -1;
				input.obs_index = -1;

				map<Input, InputData>::iterator it = input_tracker.find(input);
				if (it == input_tracker.end()) {
					InputData input_data;
					analyze_input(input,
								  scope_histories,
								  input_data);

					it = input_tracker.insert({input, input_data}).first;
				}

				if (it->second.hit_percent >= MIN_CONSIDER_HIT_PERCENT
						&& it->second.standard_deviation > 0.0) {
					double curr_val;
					if (branch_node_history->is_branch) {
						curr_val = 1.0;
					} else {
						curr_val = -1.0;
					}
					double curr_t_score = abs(curr_val - it->second.average) / it->second.standard_deviation;
					if (curr_t_score > highest_t_scores.back()) {
						highest_t.back() = input;
						highest_t_scores.back() = curr_t_score;

						int curr_index = INPUT_NUM_HIGHEST-2;
						while (true) {
							if (curr_index < 0) {
								break;
							}

							if (highest_t_scores[curr_index + 1] > highest_t_scores[curr_index]) {
								Input temp = highest_t[curr_index + 1];
								double temp_score = highest_t_scores[curr_index + 1];
								highest_t[curr_index + 1] = highest_t[curr_index];
								highest_t_scores[curr_index + 1] = highest_t_scores[curr_index];
								highest_t[curr_index] = temp;
								highest_t_scores[curr_index] = temp_score;

								curr_index--;
							} else {
								break;
							}
						}
					}
				}

				scope_context.pop_back();
				node_context.pop_back();
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)node;
				ObsNodeHistory* obs_node_history = (ObsNodeHistory*)it->second;

				for (int o_index = 0; o_index < (int)obs_node_history->obs_history.size(); o_index++) {
					scope_context.push_back(scope);
					node_context.push_back(it->first);

					Input input;
					input.scope_context = scope_context;
					input.node_context = node_context;
					input.factor_index = -1;
					input.obs_index = o_index;

					map<Input, InputData>::iterator it = input_tracker.find(input);
					if (it == input_tracker.end()) {
						InputData input_data;
						analyze_input(input,
									  scope_histories,
									  input_data);

						it = input_tracker.insert({input, input_data}).first;
					}

					if (it->second.hit_percent >= MIN_CONSIDER_HIT_PERCENT
							&& it->second.standard_deviation > 0.0) {
						double curr_val = obs_node_history->obs_history[o_index];
						double curr_t_score = abs(curr_val - it->second.average) / it->second.standard_deviation;
						if (curr_t_score > highest_t_scores.back()) {
							highest_t.back() = input;
							highest_t_scores.back() = curr_t_score;

							int curr_index = INPUT_NUM_HIGHEST-2;
							while (true) {
								if (curr_index < 0) {
									break;
								}

								if (highest_t_scores[curr_index + 1] > highest_t_scores[curr_index]) {
									Input temp = highest_t[curr_index + 1];
									double temp_score = highest_t_scores[curr_index + 1];
									highest_t[curr_index + 1] = highest_t[curr_index];
									highest_t_scores[curr_index + 1] = highest_t_scores[curr_index];
									highest_t[curr_index] = temp;
									highest_t_scores[curr_index] = temp_score;

									curr_index--;
								} else {
									break;
								}
							}
						}
					}

					scope_context.pop_back();
					node_context.pop_back();
				}

				for (int f_index = 0; f_index < (int)obs_node->factors.size(); f_index++) {
					scope_context.push_back(scope);
					node_context.push_back(it->first);

					Input input;
					input.scope_context = scope_context;
					input.node_context = node_context;
					input.factor_index = f_index;
					input.obs_index = -1;

					map<Input, InputData>::iterator it = input_tracker.find(input);
					if (it == input_tracker.end()) {
						InputData input_data;
						analyze_input(input,
									  scope_histories,
									  input_data);

						it = input_tracker.insert({input, input_data}).first;
					}

					if (it->second.hit_percent >= MIN_CONSIDER_HIT_PERCENT
							&& it->second.standard_deviation > 0.0) {
						double curr_val = obs_node_history->factor_values[f_index];
						double curr_t_score = abs(curr_val - it->second.average) / it->second.standard_deviation;
						if (curr_t_score > highest_t_scores.back()) {
							highest_t.back() = input;
							highest_t_scores.back() = curr_t_score;

							int curr_index = INPUT_NUM_HIGHEST-2;
							while (true) {
								if (curr_index < 0) {
									break;
								}

								if (highest_t_scores[curr_index + 1] > highest_t_scores[curr_index]) {
									Input temp = highest_t[curr_index + 1];
									double temp_score = highest_t_scores[curr_index + 1];
									highest_t[curr_index + 1] = highest_t[curr_index];
									highest_t_scores[curr_index + 1] = highest_t_scores[curr_index];
									highest_t[curr_index] = temp;
									highest_t_scores[curr_index] = temp_score;

									curr_index--;
								} else {
									break;
								}
							}
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

void random_input_helper(ScopeHistory* scope_history,
						 vector<Scope*>& scope_context,
						 vector<int>& node_context,
						 int& input_count,
						 Input& selected_input,
						 vector<ScopeHistory*>& scope_histories,
						 map<Input, InputData>& input_tracker) {
	Scope* scope = scope_history->scope;

	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		AbstractNode* node = it->second->node;
		switch (node->type) {
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;

				scope_context.push_back(scope);
				node_context.push_back(it->first);

				random_input_helper(scope_node_history->scope_history,
									scope_context,
									node_context,
									input_count,
									selected_input,
									scope_histories,
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
				input.node_context = node_context;
				input.factor_index = -1;
				input.obs_index = -1;

				map<Input, InputData>::iterator it = input_tracker.find(input);
				if (it == input_tracker.end()) {
					InputData input_data;
					analyze_input(input,
								  scope_histories,
								  input_data);

					it = input_tracker.insert({input, input_data}).first;
				}

				if (it->second.hit_percent >= MIN_CONSIDER_HIT_PERCENT
						&& it->second.standard_deviation > 0.0) {
					uniform_int_distribution<int> select_distribution(0, input_count);
					input_count++;
					if (select_distribution(generator) == 0) {
						selected_input = input;
					}
				}

				scope_context.pop_back();
				node_context.pop_back();
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)node;
				ObsNodeHistory* obs_node_history = (ObsNodeHistory*)it->second;

				for (int o_index = 0; o_index < (int)obs_node_history->obs_history.size(); o_index++) {
					scope_context.push_back(scope);
					node_context.push_back(it->first);

					Input input;
					input.scope_context = scope_context;
					input.node_context = node_context;
					input.factor_index = -1;
					input.obs_index = o_index;

					map<Input, InputData>::iterator it = input_tracker.find(input);
					if (it == input_tracker.end()) {
						InputData input_data;
						analyze_input(input,
									  scope_histories,
									  input_data);

						it = input_tracker.insert({input, input_data}).first;
					}

					if (it->second.hit_percent >= MIN_CONSIDER_HIT_PERCENT
							&& it->second.standard_deviation > 0.0) {
						uniform_int_distribution<int> select_distribution(0, input_count);
						input_count++;
						if (select_distribution(generator) == 0) {
							selected_input = input;
						}
					}

					scope_context.pop_back();
					node_context.pop_back();
				}

				for (int f_index = 0; f_index < (int)obs_node->factors.size(); f_index++) {
					scope_context.push_back(scope);
					node_context.push_back(it->first);

					Input input;
					input.scope_context = scope_context;
					input.node_context = node_context;
					input.factor_index = f_index;
					input.obs_index = -1;

					map<Input, InputData>::iterator it = input_tracker.find(input);
					if (it == input_tracker.end()) {
						InputData input_data;
						analyze_input(input,
									  scope_histories,
									  input_data);

						it = input_tracker.insert({input, input_data}).first;
					}

					if (it->second.hit_percent >= MIN_CONSIDER_HIT_PERCENT
							&& it->second.standard_deviation > 0.0) {
						uniform_int_distribution<int> select_distribution(0, input_count);
						input_count++;
						if (select_distribution(generator) == 0) {
							selected_input = input;
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

bool train_helper(vector<ScopeHistory*>& scope_histories,
				  vector<double>& target_val_histories,
				  double& average_score,
				  vector<Input>& factor_inputs,
				  vector<double>& factor_input_averages,
				  vector<double>& factor_input_standard_deviations,
				  vector<double>& factor_weights,
				  AbstractNode* node_context,
				  AbstractExperiment* experiment,
				  double& select_percentage) {
	int num_instances = (int)target_val_histories.size();
	int num_train_instances = (double)num_instances * (1.0 - TEST_SAMPLES_PERCENTAGE);
	int num_test_instances = num_instances - num_train_instances;

	double sum_score = 0.0;
	for (int i_index = 0; i_index < num_instances; i_index++) {
		sum_score += target_val_histories[i_index];
	}
	average_score = sum_score / (double)num_instances;

	map<Input, InputData> input_tracker;

	vector<int> factor_best_indexes;
	factor_best_helper(target_val_histories,
					   factor_best_indexes);
	vector<int> factor_worst_indexes;
	factor_worst_helper(target_val_histories,
						factor_worst_indexes);

	set<Input> s_factor_inputs;
	for (int i_index = 0; i_index < (int)factor_best_indexes.size(); i_index++) {
		vector<Input> highest_t(FACTOR_NUM_PER_GATHER);
		vector<double> highest_t_scores(FACTOR_NUM_PER_GATHER, 0.0);

		vector<Scope*> scope_context;
		vector<int> node_context;
		highest_t_factor_helper(scope_histories[factor_best_indexes[i_index]],
								scope_context,
								node_context,
								highest_t,
								highest_t_scores,
								scope_histories,
								input_tracker);

		for (int f_index = 0; f_index < (int)highest_t.size(); f_index++) {
			if (highest_t_scores[f_index] != 0.0) {
				s_factor_inputs.insert(highest_t[f_index]);
			}
		}
	}
	for (int i_index = 0; i_index < (int)factor_worst_indexes.size(); i_index++) {
		vector<Input> highest_t(FACTOR_NUM_PER_GATHER);
		vector<double> highest_t_scores(FACTOR_NUM_PER_GATHER, 0.0);

		vector<Scope*> scope_context;
		vector<int> node_context;
		highest_t_factor_helper(scope_histories[factor_worst_indexes[i_index]],
								scope_context,
								node_context,
								highest_t,
								highest_t_scores,
								scope_histories,
								input_tracker);

		for (int f_index = 0; f_index < (int)highest_t.size(); f_index++) {
			if (highest_t_scores[f_index] != 0.0) {
				s_factor_inputs.insert(highest_t[f_index]);
			}
		}
	}

	vector<double> remaining_scores(num_instances);
	vector<double> sum_vals(num_instances);

	if (s_factor_inputs.size() > 0) {
		for (set<Input>::iterator it = s_factor_inputs.begin();
				it != s_factor_inputs.end(); it++) {
			factor_inputs.push_back(*it);

			InputData input_data = input_tracker[*it];
			factor_input_averages.push_back(input_data.average);
			factor_input_standard_deviations.push_back(input_data.standard_deviation);
		}

		vector<vector<double>> factor_vals(num_instances);
		vector<vector<bool>> factor_is_on(num_instances);
		for (int h_index = 0; h_index < num_instances; h_index++) {
			vector<double> curr_factor_vals(factor_inputs.size());
			vector<bool> curr_factor_is_on(factor_inputs.size());
			for (int f_index = 0; f_index < (int)factor_inputs.size(); f_index++) {
				double val;
				bool is_on;
				fetch_input_helper(scope_histories[h_index],
								   factor_inputs[f_index],
								   0,
								   val,
								   is_on);
				curr_factor_vals[f_index] = val;
				curr_factor_is_on[f_index] = is_on;
			}
			factor_vals[h_index] = curr_factor_vals;
			factor_is_on[h_index] = curr_factor_is_on;
		}

		#if defined(MDEBUG) && MDEBUG
		#else
		double sum_offset = 0.0;
		for (int i_index = 0; i_index < num_train_instances; i_index++) {
			sum_offset += abs(target_val_histories[i_index] - average_score);
		}
		double average_offset = sum_offset / num_train_instances;
		#endif /* MDEBUG */

		Eigen::MatrixXd inputs(num_train_instances, factor_inputs.size());
		for (int i_index = 0; i_index < num_train_instances; i_index++) {
			for (int f_index = 0; f_index < (int)factor_inputs.size(); f_index++) {
				if (factor_is_on[i_index][f_index]) {
					double normalized_val = (factor_vals[i_index][f_index] - factor_input_averages[f_index]) / factor_input_standard_deviations[f_index];
					inputs(i_index, f_index) = normalized_val;
				} else {
					inputs(i_index, f_index) = 0.0;
				}
			}
		}

		Eigen::VectorXd outputs(num_train_instances);
		for (int i_index = 0; i_index < num_train_instances; i_index++) {
			outputs(i_index) = target_val_histories[i_index] - average_score;
		}

		Eigen::VectorXd weights;
		try {
			weights = inputs.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(outputs);
		} catch (std::invalid_argument &e) {
			cout << "Eigen error" << endl;
			weights = Eigen::VectorXd(factor_inputs.size());
			for (int f_index = 0; f_index < (int)factor_inputs.size(); f_index++) {
				weights(f_index) = 0.0;
			}
		}

		for (int f_index = 0; f_index < (int)factor_inputs.size(); f_index++) {
			factor_weights.push_back(weights(f_index));
		}

		#if defined(MDEBUG) && MDEBUG
		#else
		double impact_threshold = average_offset * FACTOR_IMPACT_THRESHOLD;
		#endif /* MDEBUG */

		for (int f_index = (int)factor_inputs.size() - 1; f_index >= 0; f_index--) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			double sum_impact = 0.0;
			for (int i_index = 0; i_index < num_train_instances; i_index++) {
				if (factor_is_on[i_index][f_index]) {
					double normalized_val = (factor_vals[i_index][f_index] - factor_input_averages[f_index]) / factor_input_standard_deviations[f_index];
					sum_impact += abs(normalized_val);
				}
			}

			double impact = abs(factor_weights[f_index]) * sum_impact / num_train_instances;
			if (impact < impact_threshold
					|| abs(factor_weights[f_index]) > REGRESSION_WEIGHT_LIMIT) {
			#endif /* MDEBUG */
				factor_inputs.erase(factor_inputs.begin() + f_index);
				factor_input_averages.erase(factor_input_averages.begin() + f_index);
				factor_input_standard_deviations.erase(factor_input_standard_deviations.begin() + f_index);
				factor_weights.erase(factor_weights.begin() + f_index);

				for (int i_index = 0; i_index < num_instances; i_index++) {
					factor_vals[i_index].erase(factor_vals[i_index].begin() + f_index);
					factor_is_on[i_index].erase(factor_is_on[i_index].begin() + f_index);
				}
			}
		}

		for (int i_index = 0; i_index < num_instances; i_index++) {
			double sum_score = 0.0;
			for (int f_index = 0; f_index < (int)factor_inputs.size(); f_index++) {
				if (factor_is_on[i_index][f_index]) {
					double normalized_val = (factor_vals[i_index][f_index] - factor_input_averages[f_index]) / factor_input_standard_deviations[f_index];
					sum_score += factor_weights[f_index] * normalized_val;
				}
			}

			#if defined(MDEBUG) && MDEBUG
			#else
			if (abs(sum_score) > REGRESSION_FAIL_MULTIPLIER * average_offset) {
				return false;
			}
			#endif /* MDEBUG */

			remaining_scores[i_index] = target_val_histories[i_index]
				- average_score - sum_score;
			sum_vals[i_index] = average_score + sum_score;
		}
	} else {
		for (int i_index = 0; i_index < num_instances; i_index++) {
			remaining_scores[i_index] = target_val_histories[i_index] - average_score;
			sum_vals[i_index] = average_score;
		}
	}

	int best_index = 0;
	double best_score = remaining_scores[0];
	for (int h_index = 1; h_index < num_instances; h_index++) {
		if (remaining_scores[h_index] > best_score) {
			best_index = h_index;
			best_score = remaining_scores[h_index];
		}
	}
	int worst_index = 0;
	double worst_score = remaining_scores[0];
	for (int h_index = 1; h_index < num_instances; h_index++) {
		if (remaining_scores[h_index] < worst_score) {
			worst_index = h_index;
			worst_score = remaining_scores[h_index];
		}
	}

	set<Input> s_network_inputs;
	{
		vector<Input> highest_t(INPUT_NUM_HIGHEST);
		vector<double> highest_t_scores(INPUT_NUM_HIGHEST, 0.0);

		vector<Scope*> scope_context;
		vector<int> node_context;
		highest_t_input_helper(scope_histories[best_index],
							   scope_context,
							   node_context,
							   highest_t,
							   highest_t_scores,
							   scope_histories,
							   input_tracker);

		for (int f_index = 0; f_index < (int)highest_t.size(); f_index++) {
			if (highest_t_scores[f_index] != 0.0) {
				s_network_inputs.insert(highest_t[f_index]);
			}
		}
	}
	for (int i_index = 0; i_index < INPUT_NUM_RANDOM; i_index++) {
		vector<Scope*> scope_context;
		vector<int> node_context;
		int input_count = 0;
		Input selected_input;
		random_input_helper(scope_histories[best_index],
							scope_context,
							node_context,
							input_count,
							selected_input,
							scope_histories,
							input_tracker);
		if (selected_input.scope_context.size() != 0) {
			s_network_inputs.insert(selected_input);
		}
	}
	{
		vector<Input> highest_t(INPUT_NUM_HIGHEST);
		vector<double> highest_t_scores(INPUT_NUM_HIGHEST, 0.0);

		vector<Scope*> scope_context;
		vector<int> node_context;
		highest_t_input_helper(scope_histories[worst_index],
							   scope_context,
							   node_context,
							   highest_t,
							   highest_t_scores,
							   scope_histories,
							   input_tracker);

		for (int f_index = 0; f_index < (int)highest_t.size(); f_index++) {
			if (highest_t_scores[f_index] != 0.0) {
				s_network_inputs.insert(highest_t[f_index]);
			}
		}
	}
	for (int i_index = 0; i_index < INPUT_NUM_RANDOM; i_index++) {
		vector<Scope*> scope_context;
		vector<int> node_context;
		int input_count = 0;
		Input selected_input;
		random_input_helper(scope_histories[worst_index],
							scope_context,
							node_context,
							input_count,
							selected_input,
							scope_histories,
							input_tracker);
		if (selected_input.scope_context.size() != 0) {
			s_network_inputs.insert(selected_input);
		}
	}

	if (s_network_inputs.size() > 0) {
		vector<Input> network_inputs;
		for (set<Input>::iterator it = s_network_inputs.begin();
				it != s_network_inputs.end(); it++) {
			network_inputs.push_back(*it);
		}

		vector<vector<double>> input_vals(num_instances);
		vector<vector<bool>> input_is_on(num_instances);
		for (int h_index = 0; h_index < num_instances; h_index++) {
			vector<double> curr_input_vals(network_inputs.size());
			vector<bool> curr_input_is_on(network_inputs.size());
			for (int i_index = 0; i_index < (int)network_inputs.size(); i_index++) {
				double val;
				bool is_on;
				fetch_input_helper(scope_histories[h_index],
								   network_inputs[i_index],
								   0,
								   val,
								   is_on);
				curr_input_vals[i_index] = val;
				curr_input_is_on[i_index] = is_on;
			}
			input_vals[h_index] = curr_input_vals;
			input_is_on[h_index] = curr_input_is_on;
		}

		double sum_misguess = 0.0;
		for (int i_index = num_train_instances; i_index < num_instances; i_index++) {
			sum_misguess += remaining_scores[i_index] * remaining_scores[i_index];
		}
		double average_misguess = sum_misguess / num_test_instances;

		double sum_misguess_variance = 0.0;
		for (int i_index = num_train_instances; i_index < num_instances; i_index++) {
			double curr_misguess = remaining_scores[i_index] * remaining_scores[i_index];
			sum_misguess_variance += (curr_misguess - average_misguess) * (curr_misguess - average_misguess);
		}
		double misguess_standard_deviation = sqrt(sum_misguess_variance / num_test_instances);
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

		double new_average_misguess;
		double new_misguess_standard_deviation;
		measure_network(input_vals,
						input_is_on,
						remaining_scores,
						new_network,
						new_average_misguess,
						new_misguess_standard_deviation);

		#if defined(MDEBUG) && MDEBUG
		if (rand()%2 == 0) {
		#else
		double new_improvement = average_misguess - new_average_misguess;
		double new_standard_deviation = min(misguess_standard_deviation, new_misguess_standard_deviation);
		double new_t_score = new_improvement / (new_standard_deviation / sqrt(num_test_instances));

		if (new_t_score > 2.326) {
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
				double remove_t_score = remove_improvement / (remove_standard_deviation / sqrt(num_instances * TEST_SAMPLES_PERCENTAGE));

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
				if (node_context->type == NODE_TYPE_OBS) {
					ObsNode* obs_node = (ObsNode*)node_context;

					obs_node->factors.push_back(new_factor);

					Input new_input;
					new_input.scope_context = {obs_node->parent};
					new_input.node_context = {obs_node->id};
					new_input.factor_index = (int)obs_node->factors.size()-1;
					new_input.obs_index = -1;
					factor_inputs.push_back(new_input);
					factor_input_averages.push_back(0.0);
					factor_input_standard_deviations.push_back(1.0);
					factor_weights.push_back(1.0);
				} else {
					ObsNode* new_obs_node = new ObsNode();
					new_obs_node->parent = experiment->scope_context;
					new_obs_node->id = experiment->scope_context->node_counter;
					experiment->scope_context->node_counter++;
					experiment->scope_context->nodes[new_obs_node->id] = new_obs_node;

					new_obs_node->factors.push_back(new_factor);

					switch (experiment->node_context->type) {
					case NODE_TYPE_ACTION:
						{
							ActionNode* action_node = (ActionNode*)experiment->node_context;

							new_obs_node->next_node_id = action_node->next_node_id;
							new_obs_node->next_node = action_node->next_node;

							for (int a_index = 0; a_index < (int)action_node->next_node->ancestor_ids.size(); a_index++) {
								if (action_node->next_node->ancestor_ids[a_index] == action_node->id) {
									action_node->next_node->ancestor_ids.erase(
										action_node->next_node->ancestor_ids.begin() + a_index);
									break;
								}
							}
							action_node->next_node->ancestor_ids.push_back(new_obs_node->id);

							action_node->next_node_id = new_obs_node->id;
							action_node->next_node = new_obs_node;

							new_obs_node->ancestor_ids.push_back(action_node->id);

							new_obs_node->average_hits_per_run = action_node->average_hits_per_run;
							new_obs_node->average_score = action_node->average_score;
						}
						break;
					case NODE_TYPE_SCOPE:
						{
							ScopeNode* scope_node = (ScopeNode*)experiment->node_context;

							new_obs_node->next_node_id = scope_node->next_node_id;
							new_obs_node->next_node = scope_node->next_node;

							for (int a_index = 0; a_index < (int)scope_node->next_node->ancestor_ids.size(); a_index++) {
								if (scope_node->next_node->ancestor_ids[a_index] == scope_node->id) {
									scope_node->next_node->ancestor_ids.erase(
										scope_node->next_node->ancestor_ids.begin() + a_index);
									break;
								}
							}
							scope_node->next_node->ancestor_ids.push_back(new_obs_node->id);

							scope_node->next_node_id = new_obs_node->id;
							scope_node->next_node = new_obs_node;

							new_obs_node->ancestor_ids.push_back(scope_node->id);

							new_obs_node->average_hits_per_run = scope_node->average_hits_per_run;
							new_obs_node->average_score = scope_node->average_score;
						}
						break;
					case NODE_TYPE_BRANCH:
						{
							BranchNode* branch_node = (BranchNode*)experiment->node_context;

							if (experiment->is_branch) {
								new_obs_node->next_node_id = branch_node->branch_next_node_id;
								new_obs_node->next_node = branch_node->branch_next_node;

								for (int a_index = 0; a_index < (int)branch_node->branch_next_node->ancestor_ids.size(); a_index++) {
									if (branch_node->branch_next_node->ancestor_ids[a_index] == branch_node->id) {
										branch_node->branch_next_node->ancestor_ids.erase(
											branch_node->branch_next_node->ancestor_ids.begin() + a_index);
										break;
									}
								}
								branch_node->branch_next_node->ancestor_ids.push_back(new_obs_node->id);

								branch_node->branch_next_node_id = new_obs_node->id;
								branch_node->branch_next_node = new_obs_node;

								new_obs_node->average_hits_per_run = branch_node->branch_average_hits_per_run;
								new_obs_node->average_score = branch_node->branch_average_score;
							} else {
								new_obs_node->next_node_id = branch_node->original_next_node_id;
								new_obs_node->next_node = branch_node->original_next_node;

								for (int a_index = 0; a_index < (int)branch_node->original_next_node->ancestor_ids.size(); a_index++) {
									if (branch_node->original_next_node->ancestor_ids[a_index] == branch_node->id) {
										branch_node->original_next_node->ancestor_ids.erase(
											branch_node->original_next_node->ancestor_ids.begin() + a_index);
										break;
									}
								}
								branch_node->original_next_node->ancestor_ids.push_back(new_obs_node->id);

								branch_node->original_next_node_id = new_obs_node->id;
								branch_node->original_next_node = new_obs_node;

								new_obs_node->average_hits_per_run = branch_node->original_average_hits_per_run;
								new_obs_node->average_score = branch_node->original_average_score;
							}

							new_obs_node->ancestor_ids.push_back(branch_node->id);
						}
						break;
					}

					experiment->node_context->experiment = NULL;

					experiment->node_context = new_obs_node;
					experiment->is_branch = false;
					experiment->node_context->experiment = experiment;

					Input new_input;
					new_input.scope_context = {experiment->scope_context};
					new_input.node_context = {new_obs_node->id};
					new_input.factor_index = 0;
					new_input.obs_index = -1;
					factor_inputs.push_back(new_input);
					factor_input_averages.push_back(0.0);
					factor_input_standard_deviations.push_back(1.0);
					factor_weights.push_back(1.0);
				}

				for (int i_index = 0; i_index < num_instances; i_index++) {
					new_network->activate(input_vals[i_index],
										  input_is_on[i_index]);
					sum_vals[i_index] += new_network->output->acti_vals[0];
				}
			} else {
				delete new_network;
			}
		} else {
			delete new_network;
		}
	}

	#if defined(MDEBUG) && MDEBUG
	if (rand()%2 == 0) {
		select_percentage = 0.5;
	} else {
		select_percentage = 0.0;
	}
	#else
	int num_positive = 0;
	for (int i_index = 0; i_index < num_instances; i_index++) {
		if (sum_vals[i_index] > 0.0) {
			num_positive++;
		}
	}
	select_percentage = (double)num_positive / (double)num_instances;
	#endif /* MDEBUG */

	return true;
}
