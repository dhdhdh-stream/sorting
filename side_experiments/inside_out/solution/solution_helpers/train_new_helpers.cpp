#include "solution_helpers.h"

#include <algorithm>
#include <chrono>
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
#include "solution_wrapper.h"

using namespace std;

void new_gather_factor_t_scores_helper(ScopeHistory* scope_history,
									   vector<Scope*>& scope_context,
									   vector<int>& node_context,
									   map<Input, double>& t_scores,
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

				new_gather_factor_t_scores_helper(scope_node_history->scope_history,
												  scope_context,
												  node_context,
												  t_scores,
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
						double curr_t_score = (curr_val - it->second.average) / it->second.standard_deviation;
						t_scores[input] = curr_t_score;
					}

					scope_context.pop_back();
					node_context.pop_back();
				}
			}
			break;
		}
	}
}

void new_gather_input_t_scores_helper(ScopeHistory* scope_history,
									  vector<Scope*>& scope_context,
									  vector<int>& node_context,
									  map<Input, double>& t_scores,
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

				new_gather_input_t_scores_helper(scope_node_history->scope_history,
												 scope_context,
												 node_context,
												 t_scores,
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
					double curr_t_score = (curr_val - it->second.average) / it->second.standard_deviation;
					t_scores[input] = curr_t_score;
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
						double curr_t_score = (curr_val - it->second.average) / it->second.standard_deviation;
						t_scores[input] = curr_t_score;
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
						double curr_t_score = (curr_val - it->second.average) / it->second.standard_deviation;
						t_scores[input] = curr_t_score;
					}

					scope_context.pop_back();
					node_context.pop_back();
				}
			}
			break;
		}
	}
}

void new_random_input_helper(ScopeHistory* scope_history,
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

				new_random_input_helper(scope_node_history->scope_history,
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

bool train_new(vector<ScopeHistory*>& scope_histories,
			   vector<double>& target_val_histories,
			   double& average_score,
			   vector<Input>& factor_inputs,
			   vector<double>& factor_input_averages,
			   vector<double>& factor_input_standard_deviations,
			   vector<double>& factor_weights,
			   vector<Input>& network_inputs,
			   Network*& network,
			   double& select_percentage) {
	auto start_time = chrono::high_resolution_clock::now();

	int num_instances = (int)target_val_histories.size();

	double sum_score = 0.0;
	for (int i_index = 0; i_index < num_instances; i_index++) {
		sum_score += target_val_histories[i_index];
	}
	average_score = sum_score / (double)num_instances;

	map<Input, InputData> input_tracker;

	int best_factor_index = 0;
	double best_factor_score = target_val_histories[0];
	for (int h_index = 1; h_index < num_instances; h_index++) {
		if (target_val_histories[h_index] > best_factor_score) {
			best_factor_index = h_index;
			best_factor_score = target_val_histories[h_index];
		}
	}
	int worst_factor_index = 0;
	double worst_factor_score = target_val_histories[0];
	for (int h_index = 1; h_index < num_instances; h_index++) {
		if (target_val_histories[h_index] < worst_factor_score) {
			worst_factor_index = h_index;
			worst_factor_score = target_val_histories[h_index];
		}
	}

	map<Input, double> best_factor_t_scores;
	{
		vector<Scope*> scope_context;
		vector<int> node_context;
		new_gather_factor_t_scores_helper(scope_histories[best_factor_index],
										  scope_context,
										  node_context,
										  best_factor_t_scores,
										  scope_histories,
										  input_tracker);
	}

	map<Input, double> worst_factor_t_scores;
	{
		vector<Scope*> scope_context;
		vector<int> node_context;
		new_gather_factor_t_scores_helper(scope_histories[worst_factor_index],
										  scope_context,
										  node_context,
										  worst_factor_t_scores,
										  scope_histories,
										  input_tracker);
	}

	map<Input, double> contrast_factor_t_scores;
	for (map<Input, double>::iterator best_it = best_factor_t_scores.begin();
			best_it != best_factor_t_scores.end(); best_it++) {
		map<Input, double>::iterator worst_it = worst_factor_t_scores.find(best_it->first);
		if (worst_it == worst_factor_t_scores.end()) {
			contrast_factor_t_scores[best_it->first] = abs(best_it->second);
		} else {
			contrast_factor_t_scores[best_it->first] = abs(best_it->second - worst_it->second);
		}
	}
	for (map<Input, double>::iterator worst_it = worst_factor_t_scores.begin();
			worst_it != worst_factor_t_scores.end(); worst_it++) {
		map<Input, double>::iterator best_it = best_factor_t_scores.find(worst_it->first);
		if (best_it == best_factor_t_scores.end()) {
			contrast_factor_t_scores[worst_it->first] = abs(worst_it->second);
		}
	}

	factor_inputs = vector<Input>(NUM_FACTORS);
	vector<double> factor_input_contrasts(NUM_FACTORS, 0.0);
	for (map<Input, double>::iterator it = contrast_factor_t_scores.begin();
			it != contrast_factor_t_scores.end(); it++) {
		if (it->second > factor_input_contrasts.back()) {
			factor_inputs.back() = it->first;
			factor_input_contrasts.back() = it->second;

			int curr_index = NUM_FACTORS-2;
			while (true) {
				if (curr_index < 0) {
					break;
				}

				if (factor_input_contrasts[curr_index + 1] > factor_input_contrasts[curr_index]) {
					Input temp = factor_inputs[curr_index + 1];
					double temp_score = factor_input_contrasts[curr_index + 1];
					factor_inputs[curr_index + 1] = factor_inputs[curr_index];
					factor_input_contrasts[curr_index + 1] = factor_input_contrasts[curr_index];
					factor_inputs[curr_index] = temp;
					factor_input_contrasts[curr_index] = temp_score;

					curr_index--;
				} else {
					break;
				}
			}
		}
	}
	for (int f_index = (int)factor_inputs.size()-1; f_index >= 0; f_index--) {
		if (factor_inputs[f_index].scope_context.size() == 0) {
			factor_inputs.erase(factor_inputs.begin() + f_index);
		}
	}

	vector<double> remaining_scores(num_instances);
	vector<double> sum_vals(num_instances);

	if (factor_inputs.size() > 0) {
		for (int f_index = 0; f_index < (int)factor_inputs.size(); f_index++) {
			InputData input_data = input_tracker[factor_inputs[f_index]];
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
		for (int i_index = 0; i_index < num_instances; i_index++) {
			sum_offset += abs(target_val_histories[i_index] - average_score);
		}
		double average_offset = sum_offset / (double)num_instances;
		#endif /* MDEBUG */

		Eigen::MatrixXd inputs(num_instances, factor_inputs.size());
		for (int i_index = 0; i_index < num_instances; i_index++) {
			for (int f_index = 0; f_index < (int)factor_inputs.size(); f_index++) {
				if (factor_is_on[i_index][f_index]) {
					double normalized_val = (factor_vals[i_index][f_index] - factor_input_averages[f_index]) / factor_input_standard_deviations[f_index];
					inputs(i_index, f_index) = normalized_val;
				} else {
					inputs(i_index, f_index) = 0.0;
				}
			}
		}

		Eigen::VectorXd outputs(num_instances);
		for (int i_index = 0; i_index < num_instances; i_index++) {
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
			for (int i_index = 0; i_index < num_instances; i_index++) {
				if (factor_is_on[i_index][f_index]) {
					double normalized_val = (factor_vals[i_index][f_index] - factor_input_averages[f_index]) / factor_input_standard_deviations[f_index];
					sum_impact += abs(normalized_val);
				}
			}

			double impact = abs(factor_weights[f_index]) * sum_impact / (double)num_instances;
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

	map<Input, double> best_t_scores;
	{
		vector<Scope*> scope_context;
		vector<int> node_context;
		new_gather_input_t_scores_helper(scope_histories[best_index],
										 scope_context,
										 node_context,
										 best_t_scores,
										 scope_histories,
										 input_tracker);
	}

	map<Input, double> worst_t_scores;
	{
		vector<Scope*> scope_context;
		vector<int> node_context;
		new_gather_input_t_scores_helper(scope_histories[worst_index],
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

	vector<Input> highest_inputs = vector<Input>(INPUT_NUM_HIGHEST);
	vector<double> input_contrasts(INPUT_NUM_HIGHEST, 0.0);
	for (map<Input, double>::iterator it = contrast_t_scores.begin();
			it != contrast_t_scores.end(); it++) {
		if (it->second > input_contrasts.back()) {
			highest_inputs.back() = it->first;
			input_contrasts.back() = it->second;

			int curr_index = INPUT_NUM_HIGHEST-2;
			while (true) {
				if (curr_index < 0) {
					break;
				}

				if (input_contrasts[curr_index + 1] > input_contrasts[curr_index]) {
					Input temp = highest_inputs[curr_index + 1];
					double temp_score = input_contrasts[curr_index + 1];
					highest_inputs[curr_index + 1] = highest_inputs[curr_index];
					input_contrasts[curr_index + 1] = input_contrasts[curr_index];
					highest_inputs[curr_index] = temp;
					input_contrasts[curr_index] = temp_score;

					curr_index--;
				} else {
					break;
				}
			}
		}
	}

	set<Input> s_network_inputs;
	for (int i_index = 0; i_index < (int)highest_inputs.size(); i_index++) {
		if (highest_inputs[i_index].scope_context.size() != 0) {
			s_network_inputs.insert(highest_inputs[i_index]);
		}
	}
	for (int i_index = 0; i_index < INPUT_NUM_RANDOM_PER; i_index++) {
		vector<Scope*> scope_context;
		vector<int> node_context;
		int input_count = 0;
		Input selected_input;
		new_random_input_helper(scope_histories[best_index],
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
	for (int i_index = 0; i_index < INPUT_NUM_RANDOM_PER; i_index++) {
		vector<Scope*> scope_context;
		vector<int> node_context;
		int input_count = 0;
		Input selected_input;
		new_random_input_helper(scope_histories[worst_index],
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

		network = new Network((int)network_inputs.size(),
							  input_vals,
							  input_is_on);

		train_new_network(input_vals,
						  input_is_on,
						  remaining_scores,
						  network);

		for (int i_index = 0; i_index < num_instances; i_index++) {
			network->activate(input_vals[i_index],
							  input_is_on[i_index]);
			sum_vals[i_index] += network->output->acti_vals[0];
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

	auto end_time = chrono::high_resolution_clock::now();
	auto time_diff = chrono::duration_cast<chrono::milliseconds>(end_time - start_time);
	cout << "train time: " << time_diff.count() << endl;

	return true;
}
