#include "helpers.h"

#include <cmath>
#include <iostream>

#include "branch_node.h"
#include "constants.h"
#include "factor.h"
#include "globals.h"
#include "network.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_ITERS = 30;
const int OPTIMIZE_ITERS = 10;
#else
const int TRAIN_ITERS = 300000;
const int OPTIMIZE_ITERS = 100000;
#endif /* MDEBUG */

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

void gather_t_scores_helper(ScopeHistory* scope_history,
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

				gather_t_scores_helper(scope_node_history->scope_history,
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
				input.factor_index = -1;
				input.node_context = node_context;
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
					map<Input, double>::iterator t_it = t_scores.find(input);
					if (t_it == t_scores.end()) {
						t_it = t_scores.insert({input, 0.0}).first;
					}
					t_it->second += curr_t_score;
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
						map<Input, double>::iterator t_it = t_scores.find(input);
						if (t_it == t_scores.end()) {
							t_it = t_scores.insert({input, 0.0}).first;
						}
						t_it->second += curr_t_score;
					}

					scope_context.pop_back();
					node_context.pop_back();
				}
			}
			break;
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
				double curr_val = scope_history->factor_values[f_index];
				double curr_t_score = (curr_val - it->second.average) / it->second.standard_deviation;
				map<Input, double>::iterator t_it = t_scores.find(input);
				if (t_it == t_scores.end()) {
					t_it = t_scores.insert({input, 0.0}).first;
				}
				t_it->second += curr_t_score;
			}

			scope_context.pop_back();
			node_context.pop_back();
		}
	}
}

bool is_unique(vector<vector<double>>& input_vals,
			   vector<double>& existing_averages,
			   vector<double>& existing_standard_deviations,
			   vector<double>& potential_input_vals,
			   double& potential_average,
			   double& potential_standard_deviation) {
	double sum_vals = 0.0;
	for (int h_index = 0; h_index < (int)potential_input_vals.size(); h_index++) {
		sum_vals += potential_input_vals[h_index];
	}
	potential_average = sum_vals / (double)potential_input_vals.size();

	double sum_variances = 0.0;
	for (int h_index = 0; h_index < (int)potential_input_vals.size(); h_index++) {
		sum_variances += (potential_input_vals[h_index] - potential_average)
			* (potential_input_vals[h_index] - potential_average);
	}
	potential_standard_deviation = sqrt(sum_variances / (double)potential_input_vals.size());

	for (int f_index = 0; f_index < (int)input_vals.size(); f_index++) {
		double sum_covariance = 0.0;
		for (int h_index = 0; h_index < (int)potential_input_vals.size(); h_index++) {
			sum_covariance += (potential_input_vals[h_index] - potential_average)
				* (input_vals[f_index][h_index] - existing_averages[f_index]);
		}
		double covariance = sum_covariance / (double)potential_input_vals.size();

		double pcc = covariance / potential_standard_deviation / existing_standard_deviations[f_index];
		if (abs(pcc) > UNIQUE_MAX_PCC) {
			return false;
		}
	}

	return true;
}

void train_network(vector<vector<double>>& inputs,
				   vector<vector<bool>>& input_is_on,
				   vector<double>& target_vals,
				   Network* network) {
	if (inputs.size() > 0) {
		uniform_int_distribution<int> distribution(0, inputs.size()-1);
		uniform_int_distribution<int> drop_distribution(0, 9);
		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
			int rand_index = distribution(generator);

			vector<bool> w_drop(inputs[rand_index].size());
			for (int i_index = 0; i_index < (int)inputs[rand_index].size(); i_index++) {
				if (drop_distribution(generator) == 0) {
					w_drop[i_index] = false;
				} else {
					w_drop[i_index] = input_is_on[rand_index][i_index];
				}
			}

			network->activate(inputs[rand_index],
							  w_drop);

			double error = target_vals[rand_index] - network->output->acti_vals[0];

			network->backprop(error);
		}
	}
}

void measure_network(vector<vector<double>>& inputs,
					 vector<vector<bool>>& input_is_on,
					 vector<double>& target_vals,
					 Network* network,
					 double& average_misguess,
					 double& misguess_standard_deviation) {
	vector<double> predicted_outputs(inputs.size());
	for (int d_index = 0; d_index < (int)inputs.size(); d_index++) {
		network->activate(inputs[d_index],
						  input_is_on[d_index]);
		predicted_outputs[d_index] = network->output->acti_vals[0];
	}

	double sum_misguess = 0.0;
	for (int d_index = 0; d_index < (int)inputs.size(); d_index++) {
		sum_misguess += (target_vals[d_index] - predicted_outputs[d_index]) * (target_vals[d_index] - predicted_outputs[d_index]);
	}
	average_misguess = sum_misguess / (double)inputs.size();

	double sum_misguess_variance = 0.0;
	for (int d_index = 0; d_index < (int)inputs.size(); d_index++) {
		double curr_misguess = (target_vals[d_index] - predicted_outputs[d_index]) * (target_vals[d_index] - predicted_outputs[d_index]);
		sum_misguess_variance += (curr_misguess - average_misguess) * (curr_misguess - average_misguess);
	}
	misguess_standard_deviation = sqrt(sum_misguess_variance / (double)inputs.size());
	if (misguess_standard_deviation < MIN_STANDARD_DEVIATION) {
		misguess_standard_deviation = MIN_STANDARD_DEVIATION;
	}
}

void optimize_network(vector<vector<double>>& inputs,
					  vector<vector<bool>>& input_is_on,
					  vector<double>& target_vals,
					  Network* network) {
	if (inputs.size() > 0) {
		uniform_int_distribution<int> distribution(0, inputs.size()-1);
		uniform_int_distribution<int> drop_distribution(0, 9);
		for (int iter_index = 0; iter_index < OPTIMIZE_ITERS; iter_index++) {
			int rand_index = distribution(generator);

			vector<bool> w_drop(inputs[rand_index].size());
			for (int i_index = 0; i_index < (int)inputs[rand_index].size(); i_index++) {
				if (drop_distribution(generator) == 0) {
					w_drop[i_index] = false;
				} else {
					w_drop[i_index] = input_is_on[rand_index][i_index];
				}
			}

			network->activate(inputs[rand_index],
							  w_drop);

			double error = target_vals[rand_index] - network->output->acti_vals[0];

			network->backprop(error);
		}
	}
}
