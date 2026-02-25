#include "signal_helpers.h"

#include <iostream>

#include "globals.h"
#include "scope.h"
#include "signal.h"
#include "signal_node.h"

using namespace std;

const int ITERS_PER_SAMPLE = 10;

void pre_signal_add_sample(ScopeHistory* scope_history,
						   vector<int>& explore_index,
						   double target_val,
						   SolutionWrapper* wrapper) {
	Signal* signal = scope_history->scope->pre_signal;

	vector<vector<double>> node_input_vals;
	vector<vector<bool>> node_input_is_on;
	for (int n_index = 0; n_index < (int)signal->nodes.size(); n_index++) {
		vector<double> input_vals(signal->nodes[n_index]->inputs.size());
		vector<bool> input_is_on(signal->nodes[n_index]->inputs.size());
		for (int i_index = 0; i_index < (int)signal->nodes[n_index]->inputs.size(); i_index++) {
			double val;
			bool is_on;
			fetch_input_helper(scope_history,
							   signal->nodes[n_index]->inputs[i_index],
							   explore_index,
							   0,
							   val,
							   is_on);
			input_vals[i_index] = val;
			input_is_on[i_index] = is_on;
		}
		node_input_vals.push_back(input_vals);
		node_input_is_on.push_back(input_is_on);
	}

	vector<vector<double>> potential_input_vals;
	vector<vector<bool>> potential_input_is_on;
	for (int p_index = 0; p_index < (int)signal->potential_inputs.size(); p_index++) {
		vector<double> input_vals(signal->potential_inputs[p_index].size());
		vector<bool> input_is_on(signal->potential_inputs[p_index].size());
		for (int i_index = 0; i_index < (int)signal->potential_inputs[p_index].size(); i_index++) {
			double val;
			bool is_on;
			fetch_input_helper(scope_history,
							   signal->potential_inputs[p_index][i_index],
							   explore_index,
							   0,
							   val,
							   is_on);
			input_vals[i_index] = val;
			input_is_on[i_index] = is_on;
		}
		potential_input_vals.push_back(input_vals);
		potential_input_is_on.push_back(input_is_on);
	}

	if (signal->existing_input_val_histories.size() < SIGNAL_NUM_SAMPLES) {
		signal->existing_input_val_histories.push_back(node_input_vals);
		signal->existing_input_is_on_histories.push_back(node_input_is_on);
		signal->existing_target_val_histories.push_back(target_val);

		signal->potential_existing_input_val_histories.push_back(potential_input_vals);
		signal->potential_existing_input_is_on_histories.push_back(potential_input_is_on);
	} else {
		signal->existing_input_val_histories[signal->existing_history_index] = node_input_vals;
		signal->existing_input_is_on_histories[signal->existing_history_index] = node_input_is_on;
		signal->existing_target_val_histories[signal->existing_history_index] = target_val;

		signal->potential_existing_input_val_histories[signal->existing_history_index] = potential_input_vals;
		signal->potential_existing_input_is_on_histories[signal->existing_history_index] = potential_input_is_on;

		signal->existing_history_index++;
		if (signal->existing_history_index >= SIGNAL_NUM_SAMPLES) {
			signal->existing_history_index = 0;
		}
	}
	signal->potential_existing_count++;

	if (signal->existing_input_val_histories.size() >= SIGNAL_NUM_SAMPLES) {
		uniform_int_distribution<int> sample_distribution(0, SIGNAL_NUM_SAMPLES-1);
		for (int s_index = 0; s_index < ITERS_PER_SAMPLE; s_index++) {
			signal->backprop_helper(true,
									sample_distribution(generator));
		}
	}

	if (signal->potential_existing_count >= SIGNAL_NUM_SAMPLES) {
		update_pre_signal(scope_history->scope,
						  wrapper);
	}
}

void post_signal_add_sample(ScopeHistory* scope_history,
							vector<int>& explore_index,
							double target_val,
							bool is_existing,
							SolutionWrapper* wrapper) {
	Signal* signal = scope_history->scope->post_signal;

	vector<vector<double>> node_input_vals;
	vector<vector<bool>> node_input_is_on;
	for (int n_index = 0; n_index < (int)signal->nodes.size(); n_index++) {
		vector<double> input_vals(signal->nodes[n_index]->inputs.size());
		vector<bool> input_is_on(signal->nodes[n_index]->inputs.size());
		for (int i_index = 0; i_index < (int)signal->nodes[n_index]->inputs.size(); i_index++) {
			double val;
			bool is_on;
			fetch_input_helper(scope_history,
							   signal->nodes[n_index]->inputs[i_index],
							   explore_index,
							   0,
							   val,
							   is_on);
			input_vals[i_index] = val;
			input_is_on[i_index] = is_on;
		}
		node_input_vals.push_back(input_vals);
		node_input_is_on.push_back(input_is_on);
	}

	vector<vector<double>> potential_input_vals;
	vector<vector<bool>> potential_input_is_on;
	for (int p_index = 0; p_index < (int)signal->potential_inputs.size(); p_index++) {
		vector<double> input_vals(signal->potential_inputs[p_index].size());
		vector<bool> input_is_on(signal->potential_inputs[p_index].size());
		for (int i_index = 0; i_index < (int)signal->potential_inputs[p_index].size(); i_index++) {
			double val;
			bool is_on;
			fetch_input_helper(scope_history,
							   signal->potential_inputs[p_index][i_index],
							   explore_index,
							   0,
							   val,
							   is_on);
			input_vals[i_index] = val;
			input_is_on[i_index] = is_on;
		}
		potential_input_vals.push_back(input_vals);
		potential_input_is_on.push_back(input_is_on);
	}

	if (is_existing) {
		if (signal->existing_input_val_histories.size() < SIGNAL_NUM_SAMPLES) {
			signal->existing_input_val_histories.push_back(node_input_vals);
			signal->existing_input_is_on_histories.push_back(node_input_is_on);
			signal->existing_target_val_histories.push_back(target_val);

			signal->potential_existing_input_val_histories.push_back(potential_input_vals);
			signal->potential_existing_input_is_on_histories.push_back(potential_input_is_on);
		} else {
			signal->existing_input_val_histories[signal->existing_history_index] = node_input_vals;
			signal->existing_input_is_on_histories[signal->existing_history_index] = node_input_is_on;
			signal->existing_target_val_histories[signal->existing_history_index] = target_val;

			signal->potential_existing_input_val_histories[signal->existing_history_index] = potential_input_vals;
			signal->potential_existing_input_is_on_histories[signal->existing_history_index] = potential_input_is_on;

			signal->existing_history_index++;
			if (signal->existing_history_index >= SIGNAL_NUM_SAMPLES) {
				signal->existing_history_index = 0;
			}
		}
		signal->potential_existing_count++;
	} else {
		if (signal->explore_input_val_histories.size() < SIGNAL_NUM_SAMPLES) {
			signal->explore_input_val_histories.push_back(node_input_vals);
			signal->explore_input_is_on_histories.push_back(node_input_is_on);
			signal->explore_target_val_histories.push_back(target_val);

			signal->potential_explore_input_val_histories.push_back(potential_input_vals);
			signal->potential_explore_input_is_on_histories.push_back(potential_input_is_on);
		} else {
			signal->explore_input_val_histories[signal->explore_history_index] = node_input_vals;
			signal->explore_input_is_on_histories[signal->explore_history_index] = node_input_is_on;
			signal->explore_target_val_histories[signal->explore_history_index] = target_val;

			signal->potential_explore_input_val_histories[signal->explore_history_index] = potential_input_vals;
			signal->potential_explore_input_is_on_histories[signal->explore_history_index] = potential_input_is_on;

			signal->explore_history_index++;
			if (signal->explore_history_index >= SIGNAL_NUM_SAMPLES) {
				signal->explore_history_index = 0;
			}
		}
		signal->potential_explore_count++;
	}

	if (signal->existing_input_val_histories.size() >= SIGNAL_NUM_SAMPLES
			&& signal->explore_input_val_histories.size() >= SIGNAL_NUM_SAMPLES) {
		uniform_int_distribution<int> is_existing_distribution(0, 1);
		uniform_int_distribution<int> sample_distribution(0, SIGNAL_NUM_SAMPLES-1);
		for (int s_index = 0; s_index < ITERS_PER_SAMPLE; s_index++) {
			signal->backprop_helper(is_existing_distribution(generator) == 0,
									sample_distribution(generator));
		}
	}

	if (signal->potential_existing_count >= SIGNAL_NUM_SAMPLES
			&& signal->potential_explore_count >= SIGNAL_NUM_SAMPLES) {
		update_post_signal(scope_history->scope,
						   wrapper);
	}
}
