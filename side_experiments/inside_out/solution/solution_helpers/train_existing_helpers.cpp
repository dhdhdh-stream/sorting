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

using namespace std;

void existing_gather_factor_t_scores_helper(
		ScopeHistory* scope_history,
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

				existing_gather_factor_t_scores_helper(
					scope_node_history->scope_history,
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
							&& it->second.standard_deviation >= MIN_STANDARD_DEVIATION) {
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

void existing_gather_factor_t_scores_top_helper(
		ScopeHistory* scope_history,
		map<Input, double>& t_scores,
		vector<ScopeHistory*>& scope_histories,
		map<Input, InputData>& input_tracker,
		AbstractNode* explore_node) {
	AbstractNodeHistory* explore_node_history = scope_history->node_histories[explore_node->id];

	vector<Scope*> scope_context;
	vector<int> node_context;

	Scope* scope = scope_history->scope;

	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		if (it->second->index <= explore_node_history->index) {
			AbstractNode* node = it->second->node;
			switch (node->type) {
			case NODE_TYPE_SCOPE:
				{
					ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;

					scope_context.push_back(scope);
					node_context.push_back(it->first);

					existing_gather_factor_t_scores_helper(
						scope_node_history->scope_history,
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
								&& it->second.standard_deviation >= MIN_STANDARD_DEVIATION) {
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
}

void existing_gather_input_t_scores_helper(
		ScopeHistory* scope_history,
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

				existing_gather_input_t_scores_helper(
					scope_node_history->scope_history,
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
						&& it->second.standard_deviation >= MIN_STANDARD_DEVIATION) {
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
							&& it->second.standard_deviation >= MIN_STANDARD_DEVIATION) {
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
							&& it->second.standard_deviation >= MIN_STANDARD_DEVIATION) {
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

void existing_gather_input_t_scores_top_helper(
		ScopeHistory* scope_history,
		map<Input, double>& t_scores,
		vector<ScopeHistory*>& scope_histories,
		map<Input, InputData>& input_tracker,
		AbstractNode* explore_node) {
	AbstractNodeHistory* explore_node_history = scope_history->node_histories[explore_node->id];

	vector<Scope*> scope_context;
	vector<int> node_context;

	Scope* scope = scope_history->scope;

	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		if (it->second->index <= explore_node_history->index) {
			AbstractNode* node = it->second->node;
			switch (node->type) {
			case NODE_TYPE_SCOPE:
				{
					ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;

					scope_context.push_back(scope);
					node_context.push_back(it->first);

					existing_gather_input_t_scores_helper(
						scope_node_history->scope_history,
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
							&& it->second.standard_deviation >= MIN_STANDARD_DEVIATION) {
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
								&& it->second.standard_deviation >= MIN_STANDARD_DEVIATION) {
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
								&& it->second.standard_deviation >= MIN_STANDARD_DEVIATION) {
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
}

bool train_existing(vector<ScopeHistory*>& scope_histories,
					vector<double>& target_val_histories,
					double& average_score,
					vector<Input>& factor_inputs,
					vector<double>& factor_input_averages,
					vector<double>& factor_input_standard_deviations,
					vector<double>& factor_weights,
					AbstractExperiment* experiment) {
	auto start_time = chrono::high_resolution_clock::now();

	int num_instances = (int)target_val_histories.size();
	int num_train_instances = (double)num_instances * (1.0 - TEST_SAMPLES_PERCENTAGE);
	int num_test_instances = num_instances - num_train_instances;

	map<Input, InputData> input_tracker;

	uniform_int_distribution<int> select_distribution(0, 1);

	int best_factor_index = 0;
	double best_factor_score = target_val_histories[0];
	for (int h_index = 1; h_index < num_instances; h_index++) {
		if (target_val_histories[h_index] > best_factor_score) {
			if (select_distribution(generator)) {
				best_factor_index = h_index;
				best_factor_score = target_val_histories[h_index];
			}
		}
	}
	int worst_factor_index = 0;
	double worst_factor_score = target_val_histories[0];
	for (int h_index = 1; h_index < num_instances; h_index++) {
		if (target_val_histories[h_index] < worst_factor_score) {
			if (select_distribution(generator)) {
				worst_factor_index = h_index;
				worst_factor_score = target_val_histories[h_index];
			}
		}
	}

	map<Input, double> best_factor_t_scores;
	{
		existing_gather_factor_t_scores_top_helper(
			scope_histories[best_factor_index],
			best_factor_t_scores,
			scope_histories,
			input_tracker,
			experiment->node_context);
	}

	map<Input, double> worst_factor_t_scores;
	{
		existing_gather_factor_t_scores_top_helper(
			scope_histories[worst_factor_index],
			worst_factor_t_scores,
			scope_histories,
			input_tracker,
			experiment->node_context);
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

	vector<double> remaining_scores(num_instances);

	if (contrast_factor_t_scores.size() > 0) {
		vector<pair<double,Input>> s_contrast_factor_t_scores;
		for (map<Input, double>::iterator it = contrast_factor_t_scores.begin();
				it != contrast_factor_t_scores.end(); it++) {
			s_contrast_factor_t_scores.push_back({it->second, it->first});
		}
		sort(s_contrast_factor_t_scores.begin(), s_contrast_factor_t_scores.end());

		vector<vector<double>> factor_vals;
		vector<double> h_averages;
		vector<double> h_standard_deviations;
		for (int f_index = (int)s_contrast_factor_t_scores.size()-1; f_index >= 0; f_index--) {
			InputData input_data = input_tracker[s_contrast_factor_t_scores[f_index].second];

			vector<double> curr_factor_vals(num_instances);
			double curr_average = input_data.average;
			double curr_standard_deviation = input_data.standard_deviation;
			for (int h_index = 0; h_index < num_instances; h_index++) {
				double val;
				bool is_on;
				fetch_input_helper(scope_histories[h_index],
								   s_contrast_factor_t_scores[f_index].second,
								   0,
								   val,
								   is_on);
				if (is_on) {
					double normalized_val = (val - curr_average) / curr_standard_deviation;
					curr_factor_vals[h_index] = normalized_val;
				} else {
					curr_factor_vals[h_index] = 0.0;
				}
			}

			double potential_average;
			double potential_standard_deviation;
			bool should_add = is_unique(factor_vals,
										h_averages,
										h_standard_deviations,
										curr_factor_vals,
										potential_average,
										potential_standard_deviation,
										FACTOR_MAX_PCC);

			if (should_add) {
				factor_inputs.push_back(s_contrast_factor_t_scores[f_index].second);
				factor_input_averages.push_back(curr_average);
				factor_input_standard_deviations.push_back(curr_standard_deviation);

				factor_vals.push_back(curr_factor_vals);
				h_averages.push_back(potential_average);
				h_standard_deviations.push_back(potential_standard_deviation);

				if (factor_inputs.size() >= EXISTING_NUM_FACTORS) {
					break;
				}
			}
		}

		Eigen::MatrixXd inputs(num_train_instances, 1 + factor_inputs.size());
		for (int i_index = 0; i_index < num_train_instances; i_index++) {
			inputs(i_index, 0) = 1.0;
			for (int f_index = 0; f_index < (int)factor_inputs.size(); f_index++) {
				inputs(i_index, 1 + f_index) = factor_vals[f_index][i_index];
			}
		}

		Eigen::VectorXd outputs(num_train_instances);
		for (int i_index = 0; i_index < num_train_instances; i_index++) {
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
		for (int i_index = 0; i_index < num_train_instances; i_index++) {
			sum_offset += abs(predicted(i_index) - average_score);
		}
		double average_offset = sum_offset / (double)num_train_instances;
		double impact_threshold = average_offset * FACTOR_IMPACT_THRESHOLD;
		#endif /* MDEBUG */

		for (int f_index = (int)factor_inputs.size() - 1; f_index >= 0; f_index--) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			double sum_impact = 0.0;
			for (int i_index = 0; i_index < num_train_instances; i_index++) {
				sum_impact += abs(inputs(i_index, 1 + f_index));
			}

			double impact = abs(factor_weights[f_index]) * sum_impact / num_train_instances;
			if (impact < impact_threshold) {
			#endif /* MDEBUG */
				factor_inputs.erase(factor_inputs.begin() + f_index);
				factor_input_averages.erase(factor_input_averages.begin() + f_index);
				factor_input_standard_deviations.erase(factor_input_standard_deviations.begin() + f_index);
				factor_weights.erase(factor_weights.begin() + f_index);

				factor_vals.erase(factor_vals.begin() + f_index);
			}
		}

		for (int i_index = 0; i_index < num_instances; i_index++) {
			double sum_score = 0.0;
			for (int f_index = 0; f_index < (int)factor_inputs.size(); f_index++) {
				sum_score += factor_weights[f_index] * factor_vals[f_index][i_index];
			}

			#if defined(MDEBUG) && MDEBUG
			#else
			if (abs(sum_score) > REGRESSION_FAIL_MULTIPLIER * average_offset) {
				cout << "abs(sum_score): " << abs(sum_score) << endl;
				cout << "average_offset: " << average_offset << endl;
				return false;
			}
			#endif /* MDEBUG */

			remaining_scores[i_index] = target_val_histories[i_index]
				- average_score - sum_score;
		}
	} else {
		double sum_score = 0.0;
		for (int i_index = 0; i_index < num_instances; i_index++) {
			sum_score += target_val_histories[i_index];
		}
		average_score = sum_score / (double)num_instances;

		for (int i_index = 0; i_index < num_instances; i_index++) {
			remaining_scores[i_index] = target_val_histories[i_index] - average_score;
		}
	}

	int best_index = 0;
	double best_score = remaining_scores[0];
	for (int h_index = 1; h_index < num_instances; h_index++) {
		if (remaining_scores[h_index] > best_score) {
			if (select_distribution(generator)) {
				best_index = h_index;
				best_score = remaining_scores[h_index];
			}
		}
	}
	int worst_index = 0;
	double worst_score = remaining_scores[0];
	for (int h_index = 1; h_index < num_instances; h_index++) {
		if (remaining_scores[h_index] < worst_score) {
			if (select_distribution(generator)) {
				worst_index = h_index;
				worst_score = remaining_scores[h_index];
			}
		}
	}

	map<Input, double> best_t_scores;
	{
		existing_gather_input_t_scores_top_helper(
			scope_histories[best_index],
			best_t_scores,
			scope_histories,
			input_tracker,
			experiment->node_context);
	}

	map<Input, double> worst_t_scores;
	{
		existing_gather_input_t_scores_top_helper(
			scope_histories[worst_index],
			worst_t_scores,
			scope_histories,
			input_tracker,
			experiment->node_context);
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

	if (contrast_t_scores.size() > 0) {
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
										potential_standard_deviation,
										NETWORK_MAX_PCC);

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
										potential_standard_deviation,
										NETWORK_MAX_PCC);

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
				Input new_input;
				existing_add_factor(scope_histories,
									network_inputs,
									new_network,
									new_input,
									experiment);

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
	}

	auto end_time = chrono::high_resolution_clock::now();
	auto time_diff = chrono::duration_cast<chrono::milliseconds>(end_time - start_time);
	cout << "train time: " << time_diff.count() << endl;

	return true;
}
