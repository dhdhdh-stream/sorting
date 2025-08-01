#include "helpers.h"

#include <algorithm>

#include "branch_node.h"
#include "constants.h"
#include "factor.h"
#include "globals.h"
#include "network.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

const int SPLIT_NUM_INPUTS = 10;

const double SEED_RATIO = 0.1;

const int MAX_EPOCHS = 30;
const int ITERS_PER_EPOCH = 10000;
const double MAX_AVERAGE_ERROR = 0.1;

void gather_inputs_helper(ScopeHistory* scope_history,
						  vector<Scope*>& scope_context,
						  vector<int>& node_context,
						  vector<pair<Input,InputData>>& inputs,
						  vector<ScopeHistory*>& scope_histories) {
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

				gather_inputs_helper(scope_node_history->scope_history,
									 scope_context,
									 node_context,
									 inputs,
									 scope_histories);

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

				InputData input_data;
				analyze_input(input,
							  scope_histories,
							  input_data);

				if (input_data.hit_percent >= MIN_CONSIDER_HIT_PERCENT
						&& input_data.standard_deviation >= MIN_STANDARD_DEVIATION) {
					inputs.push_back({input, input_data});
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

					InputData input_data;
					analyze_input(input,
								  scope_histories,
								  input_data);

					if (input_data.hit_percent >= MIN_CONSIDER_HIT_PERCENT
							&& input_data.standard_deviation >= MIN_STANDARD_DEVIATION) {
						inputs.push_back({input, input_data});
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

			InputData input_data;
			analyze_input(input,
						  scope_histories,
						  input_data);

			if (input_data.hit_percent >= MIN_CONSIDER_HIT_PERCENT
					&& input_data.standard_deviation >= MIN_STANDARD_DEVIATION) {
				inputs.push_back({input, input_data});
			}

			scope_context.pop_back();
			node_context.pop_back();
		}
	}
}

bool split_helper(vector<ScopeHistory*>& existing_scope_histories,
				  vector<ScopeHistory*>& explore_scope_histories,
				  vector<Input>& match_inputs,
				  Network*& match_network) {
	vector<ScopeHistory*> combined_scope_histories;
	combined_scope_histories = existing_scope_histories;
	combined_scope_histories.insert(combined_scope_histories.end(),
			explore_scope_histories.begin(), explore_scope_histories.end());

	/**
	 * - simply randomly select history to gather possible inputs
	 */
	uniform_int_distribution<int> gather_input_distribution(0, existing_scope_histories.size()-1);
	ScopeHistory* gather_history = existing_scope_histories[gather_input_distribution(generator)];
	vector<Scope*> scope_context;
	vector<int> node_context;
	vector<pair<Input,InputData>> possible_inputs;
	gather_inputs_helper(gather_history,
						 scope_context,
						 node_context,
						 possible_inputs,
						 combined_scope_histories);

	/**
	 * - simply randomly select inputs
	 */
	vector<double> input_averages;
	vector<double> input_standard_deviations;
	vector<vector<double>> v_existing_input_vals;
	vector<vector<bool>> v_existing_input_is_on;
	vector<vector<double>> v_explore_input_vals;
	vector<vector<bool>> v_explore_input_is_on;
	vector<vector<double>> n_input_vals;
	vector<double> h_averages;
	vector<double> h_standard_deviations;
	while (possible_inputs.size() > 0) {
		uniform_int_distribution<int> input_distribution(0, possible_inputs.size()-1);
		int input_index = input_distribution(generator);

		Input input = possible_inputs[input_index].first;
		InputData input_data = possible_inputs[input_index].second;

		vector<double> curr_existing_input_vals(existing_scope_histories.size());
		vector<bool> curr_existing_input_is_on(existing_scope_histories.size());
		vector<double> curr_explore_input_vals(explore_scope_histories.size());
		vector<bool> curr_explore_input_is_on(explore_scope_histories.size());
		vector<double> curr_n_input_vals(combined_scope_histories.size());
		double curr_average = input_data.average;
		double curr_standard_deviation = input_data.standard_deviation;
		for (int h_index = 0; h_index < (int)existing_scope_histories.size(); h_index++) {
			double val;
			bool is_on;
			fetch_input_helper(existing_scope_histories[h_index],
							   input,
							   0,
							   val,
							   is_on);
			curr_existing_input_vals[h_index] = val;
			curr_existing_input_is_on[h_index] = is_on;
			if (is_on) {
				double normalized_val = (val - curr_average) / curr_standard_deviation;
				curr_n_input_vals[h_index] = normalized_val;
			} else {
				curr_n_input_vals[h_index] = 0.0;
			}
		}
		for (int h_index = 0; h_index < (int)explore_scope_histories.size(); h_index++) {
			double val;
			bool is_on;
			fetch_input_helper(explore_scope_histories[h_index],
							   input,
							   0,
							   val,
							   is_on);
			curr_explore_input_vals[h_index] = val;
			curr_explore_input_is_on[h_index] = is_on;
			if (is_on) {
				double normalized_val = (val - curr_average) / curr_standard_deviation;
				curr_n_input_vals[(int)explore_scope_histories.size() + h_index] = normalized_val;
			} else {
				curr_n_input_vals[(int)explore_scope_histories.size() + h_index] = 0.0;
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

		possible_inputs.erase(possible_inputs.begin() + input_index);

		if (should_add) {
			match_inputs.push_back(input);
			input_averages.push_back(curr_average);
			input_standard_deviations.push_back(curr_standard_deviation);
			v_existing_input_vals.push_back(curr_existing_input_vals);
			v_existing_input_is_on.push_back(curr_existing_input_is_on);
			v_explore_input_vals.push_back(curr_explore_input_vals);
			v_explore_input_is_on.push_back(curr_explore_input_is_on);
			n_input_vals.push_back(curr_n_input_vals);
			h_averages.push_back(potential_average);
			h_standard_deviations.push_back(potential_standard_deviation);

			if (match_inputs.size() >= SPLIT_NUM_INPUTS) {
				break;
			}
		}
	}

	match_network = new Network(match_inputs.size(),
								input_averages,
								input_standard_deviations);

	vector<int> negative_seeds;
	int num_seeds = SEED_RATIO * (double)explore_scope_histories.size();
	vector<int> initial_possible_indexes(explore_scope_histories.size());
	for (int i_index = 0; i_index < (int)explore_scope_histories.size(); i_index++) {
		initial_possible_indexes[i_index] = i_index;
	}
	for (int s_index = 0; s_index < num_seeds; s_index++) {
		uniform_int_distribution<int> possible_distribution(0, initial_possible_indexes.size()-1);
		int random_index = possible_distribution(generator);
		negative_seeds.push_back(initial_possible_indexes[random_index]);
		initial_possible_indexes.erase(initial_possible_indexes.begin() + random_index);
	}

	uniform_int_distribution<int> is_existing_distribution(0, 1);
	uniform_int_distribution<int> existing_distribution(0, existing_scope_histories.size()-1);
	uniform_int_distribution<int> seed_distribution(0, num_seeds-1);
	uniform_int_distribution<int> drop_distribution(0, 9);
	int e_index = 0;
	while (true) {
		double sum_errors = 0.0;
		for (int iter_index = 0; iter_index < ITERS_PER_EPOCH; iter_index++) {
			vector<double> inputs(match_inputs.size());
			vector<bool> input_is_on(match_inputs.size());

			bool is_existing = is_existing_distribution(generator) == 0;

			if (is_existing) {
				int random_index = existing_distribution(generator);
				for (int i_index = 0; i_index < (int)match_inputs.size(); i_index++) {
					inputs[i_index] = v_existing_input_vals[i_index][random_index];
					if (drop_distribution(generator) == 0) {
						input_is_on[i_index] = false;
					} else {
						input_is_on[i_index] = v_existing_input_is_on[i_index][random_index];
					}
				}
			} else {
				int random_index = seed_distribution(generator);
				int explore_index = negative_seeds[random_index];
				for (int i_index = 0; i_index < (int)match_inputs.size(); i_index++) {
					inputs[i_index] = v_explore_input_vals[i_index][explore_index];
					if (drop_distribution(generator) == 0) {
						input_is_on[i_index] = false;
					} else {
						input_is_on[i_index] = v_explore_input_is_on[i_index][explore_index];
					}
				}
			}

			match_network->activate(inputs,
									input_is_on);

			double error;
			if (is_existing) {
				if (match_network->output->acti_vals[0] < 1.0) {
					error = 1.0 - match_network->output->acti_vals[0];
				}
			} else {
				if (match_network->output->acti_vals[0] > -1.0) {
					error = -1.0 - match_network->output->acti_vals[0];
				}
			}

			match_network->backprop(error);

			sum_errors += abs(error);
		}

		double average_error = sum_errors / ITERS_PER_EPOCH;
		if (average_error <= MAX_AVERAGE_ERROR) {
			return true;
		}

		e_index++;
		if (e_index >= MAX_EPOCHS) {
			return false;
		}

		vector<pair<double,int>> explore_acti_vals(explore_scope_histories.size());
		for (int h_index = 0; h_index < (int)explore_scope_histories.size(); h_index++) {
			vector<double> inputs(match_inputs.size());
			vector<bool> input_is_on(match_inputs.size());
			for (int i_index = 0; i_index < (int)match_inputs.size(); i_index++) {
				inputs[i_index] = v_explore_input_vals[i_index][h_index];
				input_is_on[i_index] = v_explore_input_is_on[i_index][h_index];
			}

			match_network->activate(inputs,
									input_is_on);

			explore_acti_vals[h_index] = {match_network->output->acti_vals[0], h_index};
		}
		sort(explore_acti_vals.begin(), explore_acti_vals.end());
		for (int s_index = 0; s_index < num_seeds; s_index++) {
			negative_seeds[s_index] = explore_acti_vals[s_index].second;
		}
	}

	return true;
}
