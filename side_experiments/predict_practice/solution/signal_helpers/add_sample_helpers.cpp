#include "signal_helpers.h"

#include <iostream>

#include "globals.h"
#include "scope.h"
#include "scope_node.h"
#include "signal.h"
#include "signal_node.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_MIN_SAMPLES = 200;
#else
const int TRAIN_MIN_SAMPLES = 20000;
#endif /* MDEBUG */
const int ITERS_PER_SAMPLE = 10;

void update_signals_helper(double target_val,
						   SolutionWrapper* wrapper) {
	Signal* signal = wrapper->solution->signal;

	vector<vector<double>> node_input_vals;
	vector<vector<bool>> node_input_is_on;
	for (int n_index = 0; n_index < (int)signal->nodes.size(); n_index++) {
		vector<double> input_vals(signal->nodes[n_index]->inputs.size());
		vector<bool> input_is_on(signal->nodes[n_index]->inputs.size());
		for (int i_index = 0; i_index < (int)signal->nodes[n_index]->inputs.size(); i_index++) {
			double val;
			bool is_on;
			signal->nodes[n_index]->inputs[i_index].activate(
				wrapper->world_model,
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
			signal->potential_inputs[p_index][i_index].activate(
				wrapper->world_model,
				val,
				is_on);
			input_vals[i_index] = val;
			input_is_on[i_index] = is_on;
		}
		potential_input_vals.push_back(input_vals);
		potential_input_is_on.push_back(input_is_on);
	}

	if (signal->input_val_histories.size() < SIGNAL_NUM_SAMPLES) {
		signal->input_val_histories.push_back(node_input_vals);
		signal->input_is_on_histories.push_back(node_input_is_on);
		signal->target_val_histories.push_back(target_val);

		signal->potential_input_val_histories.push_back(potential_input_vals);
		signal->potential_input_is_on_histories.push_back(potential_input_is_on);
	} else {
		signal->input_val_histories[signal->history_index] = node_input_vals;
		signal->input_is_on_histories[signal->history_index] = node_input_is_on;
		signal->target_val_histories[signal->history_index] = target_val;

		signal->potential_input_val_histories[signal->history_index] = potential_input_vals;
		signal->potential_input_is_on_histories[signal->history_index] = potential_input_is_on;
	}
	signal->history_index++;
	if (signal->history_index >= SIGNAL_NUM_SAMPLES) {
		signal->history_index = 0;
	}

	if (signal->input_val_histories.size() >= TRAIN_MIN_SAMPLES) {
		uniform_int_distribution<int> sample_distribution(1, TRAIN_MIN_SAMPLES);
		for (int s_index = 0; s_index < ITERS_PER_SAMPLE; s_index++) {
			int index = signal->history_index - sample_distribution(generator);
			if (index < 0) {
				index += SIGNAL_NUM_SAMPLES;
			}
			signal->backprop_helper(index);
		}
	}

	signal->potential_count++;
	if (signal->potential_count >= SIGNAL_NUM_SAMPLES) {
		update_signal(wrapper->solution,
					  wrapper);
	}
}
