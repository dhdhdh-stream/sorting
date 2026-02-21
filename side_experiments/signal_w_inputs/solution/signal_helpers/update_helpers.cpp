#include "signal_helpers.h"

#include <algorithm>
#include <iostream>

#include "constants.h"
#include "globals.h"
#include "layer.h"
#include "scope.h"
#include "signal.h"
#include "signal_node.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int INIT_ITERS = 20;
const int REFINE_ITERS = 10;
const int CLEAN_ITERS = 10;
#else
const int INIT_ITERS = 200000;
const int REFINE_ITERS = 100000;
const int CLEAN_ITERS = 100000;
#endif /* MDEBUG */

const double VALIDATION_RATIO = 0.2;

void clean_signal_helper(double& existing_sum_misguess,
						 Scope* scope,
						 int node_index,
						 SolutionWrapper* wrapper) {
	Signal* curr_signal = new Signal();
	curr_signal->copy_from(scope->signal,
						   wrapper->solution);

	delete curr_signal->nodes[node_index];
	curr_signal->nodes.erase(curr_signal->nodes.begin() + node_index);

	curr_signal->output_weights.erase(curr_signal->output_weights.begin() + node_index);
	curr_signal->output_weight_updates.erase(curr_signal->output_weight_updates.begin() + node_index);

	for (int h_index = 0; h_index < SIGNAL_NUM_SAMPLES; h_index++) {
		curr_signal->existing_input_val_histories[h_index].erase(
			curr_signal->existing_input_val_histories[h_index].begin() + node_index);
		curr_signal->existing_input_is_on_histories[h_index].erase(
			curr_signal->existing_input_is_on_histories[h_index].begin() + node_index);
		curr_signal->explore_input_val_histories[h_index].erase(
			curr_signal->explore_input_val_histories[h_index].begin() + node_index);
		curr_signal->explore_input_is_on_histories[h_index].erase(
			curr_signal->explore_input_is_on_histories[h_index].begin() + node_index);
	}

	int num_train_samples = (1.0 - VALIDATION_RATIO) * SIGNAL_NUM_SAMPLES;

	uniform_int_distribution<int> existing_distribution(0, 1);
	uniform_int_distribution<int> train_distribution(0, SIGNAL_NUM_SAMPLES-1);
	for (int iter_index = 0; iter_index < CLEAN_ITERS; iter_index++) {
		if (existing_distribution(generator)) {
			int index = train_distribution(generator);
			curr_signal->backprop_helper(true,
										 index);
		} else {
			int index = train_distribution(generator);
			curr_signal->backprop_helper(false,
										 index);
		}
	}

	double existing_curr_sum_misguess = 0.0;
	for (int h_index = num_train_samples; h_index < SIGNAL_NUM_SAMPLES; h_index++) {
		double val = curr_signal->activate_helper(true,
												  h_index);

		existing_curr_sum_misguess += (curr_signal->existing_target_val_histories[h_index] - val)
			* (curr_signal->existing_target_val_histories[h_index] - val);
	}

	double explore_curr_sum_misguess = 0.0;
	for (int h_index = num_train_samples; h_index < SIGNAL_NUM_SAMPLES; h_index++) {
		double val = curr_signal->activate_helper(false,
												  h_index);

		explore_curr_sum_misguess += (curr_signal->explore_target_val_histories[h_index] - val)
			* (curr_signal->explore_target_val_histories[h_index] - val);
	}

	double curr_sum_misguess = (existing_curr_sum_misguess + explore_curr_sum_misguess) / 2.0;

	if (curr_sum_misguess < existing_sum_misguess) {
		existing_sum_misguess = curr_sum_misguess;

		delete scope->signal;
		scope->signal = curr_signal;
	} else {
		delete curr_signal;
	}
}

/**
 * - weigh existing vs. explore 1-to-1
 *   - existing may have high variance
 *   - only rely on explore to generalize slightly off of existing
 */
void update_signal(Scope* scope,
				   SolutionWrapper* wrapper) {
	Signal* signal = scope->signal;

	{
		default_random_engine generator_copy = generator;
		shuffle(signal->existing_input_val_histories.begin(), signal->existing_input_val_histories.end(), generator_copy);
	}
	{
		default_random_engine generator_copy = generator;
		shuffle(signal->existing_input_is_on_histories.begin(), signal->existing_input_is_on_histories.end(), generator_copy);
	}
	{
		default_random_engine generator_copy = generator;
		shuffle(signal->existing_target_val_histories.begin(), signal->existing_target_val_histories.end(), generator_copy);
	}
	{
		default_random_engine generator_copy = generator;
		shuffle(signal->explore_input_val_histories.begin(), signal->explore_input_val_histories.end(), generator_copy);
	}
	{
		default_random_engine generator_copy = generator;
		shuffle(signal->explore_input_is_on_histories.begin(), signal->explore_input_is_on_histories.end(), generator_copy);
	}
	{
		default_random_engine generator_copy = generator;
		shuffle(signal->explore_target_val_histories.begin(), signal->explore_target_val_histories.end(), generator_copy);
	}
	{
		default_random_engine generator_copy = generator;
		shuffle(signal->potential_existing_input_val_histories.begin(), signal->potential_existing_input_val_histories.end(), generator_copy);
	}
	{
		default_random_engine generator_copy = generator;
		shuffle(signal->potential_existing_input_is_on_histories.begin(), signal->potential_existing_input_is_on_histories.end(), generator_copy);
	}
	{
		default_random_engine generator_copy = generator;
		shuffle(signal->potential_explore_input_val_histories.begin(), signal->potential_explore_input_val_histories.end(), generator_copy);
	}
	{
		default_random_engine generator_copy = generator;
		shuffle(signal->potential_explore_input_is_on_histories.begin(), signal->potential_explore_input_is_on_histories.end(), generator_copy);
	}

	int num_train_samples = (1.0 - VALIDATION_RATIO) * SIGNAL_NUM_SAMPLES;

	if (signal->nodes.size() == 0) {
		double existing_sum_vals = 0.0;
		for (int h_index = 0; h_index < num_train_samples; h_index++) {
			existing_sum_vals += signal->existing_target_val_histories[h_index];
		}
		double existing_val_average = existing_sum_vals / num_train_samples;

		double explore_sum_vals = 0.0;
		for (int h_index = 0; h_index < num_train_samples; h_index++) {
			explore_sum_vals += signal->explore_target_val_histories[h_index];
		}
		double explore_val_average = explore_sum_vals / num_train_samples;

		signal->output_constant = (existing_val_average + explore_val_average) / 2.0;
	}

	vector<double> existing_remaining_vals(SIGNAL_NUM_SAMPLES);
	for (int h_index = 0; h_index < SIGNAL_NUM_SAMPLES; h_index++) {
		double val = signal->activate_helper(true,
											 h_index);
		existing_remaining_vals[h_index] = signal->existing_target_val_histories[h_index] - val;
	}

	vector<double> explore_remaining_vals(SIGNAL_NUM_SAMPLES);
	for (int h_index = 0; h_index < SIGNAL_NUM_SAMPLES; h_index++) {
		double val = signal->activate_helper(false,
											 h_index);
		explore_remaining_vals[h_index] = signal->explore_target_val_histories[h_index] - val;
	}

	double existing_existing_sum_misguess = 0.0;
	for (int h_index = num_train_samples; h_index < SIGNAL_NUM_SAMPLES; h_index++) {
		existing_existing_sum_misguess += existing_remaining_vals[h_index] * existing_remaining_vals[h_index];
	}

	double explore_existing_sum_misguess = 0.0;
	for (int h_index = num_train_samples; h_index < SIGNAL_NUM_SAMPLES; h_index++) {
		explore_existing_sum_misguess += explore_remaining_vals[h_index] * explore_remaining_vals[h_index];
	}

	double existing_sum_misguess = (existing_existing_sum_misguess + explore_existing_sum_misguess) / 2.0;

	Signal* best_signal = NULL;
	double best_sum_misguess = numeric_limits<double>::max();

	uniform_int_distribution<int> existing_distribution(0, 1);
	uniform_int_distribution<int> train_distribution(0, num_train_samples-1);
	for (int try_index = 0; try_index < (int)signal->potential_inputs.size(); try_index++) {
		vector<double> input_averages;
		vector<double> input_standard_deviations;
		for (int i_index = 0; i_index < (int)signal->potential_inputs[try_index].size(); i_index++) {
			double sum_vals = 0.0;
			int count = 0;
			for (int h_index = 0; h_index < num_train_samples; h_index++) {
				if (signal->potential_existing_input_is_on_histories[h_index][try_index][i_index]) {
					sum_vals += signal->potential_existing_input_val_histories[h_index][try_index][i_index];
					count++;
				}
			}
			for (int h_index = 0; h_index < num_train_samples; h_index++) {
				if (signal->potential_explore_input_is_on_histories[h_index][try_index][i_index]) {
					sum_vals += signal->potential_explore_input_val_histories[h_index][try_index][i_index];
					count++;
				}
			}
			if (count == 0) {
				input_averages.push_back(0.0);
				input_standard_deviations.push_back(1.0);
			} else {
				double val_average = sum_vals / count;
				double sum_variances = 0.0;
				for (int h_index = 0; h_index < num_train_samples; h_index++) {
					if (signal->potential_existing_input_is_on_histories[h_index][try_index][i_index]) {
						double val = signal->potential_existing_input_val_histories[h_index][try_index][i_index];
						sum_variances += (val - val_average) * (val - val_average);
					}
				}
				for (int h_index = 0; h_index < num_train_samples; h_index++) {
					if (signal->potential_explore_input_is_on_histories[h_index][try_index][i_index]) {
						double val = signal->potential_explore_input_val_histories[h_index][try_index][i_index];
						sum_variances += (val - val_average) * (val - val_average);
					}
				}
				double val_standard_deviation = sqrt(sum_variances / count);
				if (val_standard_deviation < MIN_STANDARD_DEVIATION) {
					val_standard_deviation = MIN_STANDARD_DEVIATION;
				}

				input_averages.push_back(val_average);
				input_standard_deviations.push_back(val_standard_deviation);
			}
		}

		SignalNode* curr_node = new SignalNode(signal->potential_inputs[try_index],
											   input_averages,
											   input_standard_deviations);

		/**
		 * - initially, update layers independently to drive convergence
		 *   - (stop after as unstable)
		 */
		int init_epoch_iter = 0;
		double init_hidden_1_average_max_update = 0.0;
		double init_hidden_2_average_max_update = 0.0;
		double init_hidden_3_average_max_update = 0.0;
		double init_output_average_max_update = 0.0;
		for (int iter_index = 0; iter_index < INIT_ITERS; iter_index++) {
			if (existing_distribution(generator)) {
				int index = train_distribution(generator);

				curr_node->activate(signal->potential_existing_input_val_histories[index][try_index],
									signal->potential_existing_input_is_on_histories[index][try_index]);

				double error = existing_remaining_vals[index] - curr_node->output->acti_vals[0];
				curr_node->output->errors[0] = error;
				curr_node->backprop();
			} else {
				int index = train_distribution(generator);

				curr_node->activate(signal->potential_explore_input_val_histories[index][try_index],
									signal->potential_explore_input_is_on_histories[index][try_index]);

				double error = explore_remaining_vals[index] - curr_node->output->acti_vals[0];
				curr_node->output->errors[0] = error;
				curr_node->backprop();
			}

			init_epoch_iter++;
			if (init_epoch_iter == NETWORK_EPOCH_SIZE) {
				double hidden_1_max_update = 0.0;
				curr_node->hidden_1->get_max_update(hidden_1_max_update);
				init_hidden_1_average_max_update = 0.999*init_hidden_1_average_max_update+0.001*hidden_1_max_update;
				if (hidden_1_max_update > 0.0) {
					double hidden_1_learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE)/init_hidden_1_average_max_update;
					if (hidden_1_learning_rate*hidden_1_max_update > NETWORK_TARGET_MAX_UPDATE) {
						hidden_1_learning_rate = NETWORK_TARGET_MAX_UPDATE/hidden_1_max_update;
					}
					curr_node->hidden_1->update_weights(hidden_1_learning_rate);
				}

				double hidden_2_max_update = 0.0;
				curr_node->hidden_2->get_max_update(hidden_2_max_update);
				init_hidden_2_average_max_update = 0.999*init_hidden_2_average_max_update+0.001*hidden_2_max_update;
				if (hidden_2_max_update > 0.0) {
					double hidden_2_learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE)/init_hidden_2_average_max_update;
					if (hidden_2_learning_rate*hidden_2_max_update > NETWORK_TARGET_MAX_UPDATE) {
						hidden_2_learning_rate = NETWORK_TARGET_MAX_UPDATE/hidden_2_max_update;
					}
					curr_node->hidden_2->update_weights(hidden_2_learning_rate);
				}

				double hidden_3_max_update = 0.0;
				curr_node->hidden_3->get_max_update(hidden_3_max_update);
				init_hidden_3_average_max_update = 0.999*init_hidden_3_average_max_update+0.001*hidden_3_max_update;
				if (hidden_3_max_update > 0.0) {
					double hidden_3_learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE)/init_hidden_3_average_max_update;
					if (hidden_3_learning_rate*hidden_3_max_update > NETWORK_TARGET_MAX_UPDATE) {
						hidden_3_learning_rate = NETWORK_TARGET_MAX_UPDATE/hidden_3_max_update;
					}
					curr_node->hidden_3->update_weights(hidden_3_learning_rate);
				}

				double output_max_update = 0.0;
				curr_node->output->get_max_update(output_max_update);
				init_output_average_max_update = 0.999*init_output_average_max_update+0.001*output_max_update;
				if (output_max_update > 0.0) {
					double output_learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE)/init_output_average_max_update;
					if (output_learning_rate*output_max_update > NETWORK_TARGET_MAX_UPDATE) {
						output_learning_rate = NETWORK_TARGET_MAX_UPDATE/output_max_update;
					}
					curr_node->output->update_weights(output_learning_rate);
				}

				init_epoch_iter = 0;
			}
		}

		Signal* curr_signal = new Signal();
		curr_signal->copy_from(signal,
							   wrapper->solution);

		curr_signal->nodes.push_back(curr_node);

		curr_signal->output_weights.push_back(1.0);
		curr_signal->output_weight_updates.push_back(0.0);

		for (int h_index = 0; h_index < SIGNAL_NUM_SAMPLES; h_index++) {
			curr_signal->existing_input_val_histories[h_index]
				.push_back(curr_signal->potential_existing_input_val_histories[h_index][try_index]);
			curr_signal->existing_input_is_on_histories[h_index]
				.push_back(curr_signal->potential_existing_input_is_on_histories[h_index][try_index]);
			curr_signal->explore_input_val_histories[h_index]
				.push_back(curr_signal->potential_explore_input_val_histories[h_index][try_index]);
			curr_signal->explore_input_is_on_histories[h_index]
				.push_back(curr_signal->potential_explore_input_is_on_histories[h_index][try_index]);
		}

		curr_signal->potential_inputs.clear();

		for (int iter_index = 0; iter_index < REFINE_ITERS; iter_index++) {
			if (existing_distribution(generator)) {
				int index = train_distribution(generator);
				curr_signal->backprop_helper(true,
											 index);
			} else {
				int index = train_distribution(generator);
				curr_signal->backprop_helper(false,
											 index);
			}
		}

		double existing_curr_sum_misguess = 0.0;
		for (int h_index = num_train_samples; h_index < SIGNAL_NUM_SAMPLES; h_index++) {
			double val = curr_signal->activate_helper(true,
													  h_index);

			existing_curr_sum_misguess += (curr_signal->existing_target_val_histories[h_index] - val)
				* (curr_signal->existing_target_val_histories[h_index] - val);
		}

		double explore_curr_sum_misguess = 0.0;
		for (int h_index = num_train_samples; h_index < SIGNAL_NUM_SAMPLES; h_index++) {
			double val = curr_signal->activate_helper(false,
													  h_index);

			explore_curr_sum_misguess += (curr_signal->explore_target_val_histories[h_index] - val)
				* (curr_signal->explore_target_val_histories[h_index] - val);
		}

		double curr_sum_misguess = (existing_curr_sum_misguess + explore_curr_sum_misguess) / 2.0;

		if (curr_sum_misguess < existing_sum_misguess && curr_sum_misguess < best_sum_misguess) {
			if (best_signal != NULL) {
				delete best_signal;
			}
			best_signal = curr_signal;
			best_sum_misguess = curr_sum_misguess;
		} else {
			delete curr_signal;
		}
	}

	if (best_sum_misguess < existing_sum_misguess) {
		existing_sum_misguess = best_sum_misguess;

		delete scope->signal;
		scope->signal = best_signal;
	}

	scope->signal->potential_inputs.clear();

	for (int n_index = (int)scope->signal->nodes.size()-1; n_index >= 0; n_index--) {
		clean_signal_helper(existing_sum_misguess,
							scope,
							n_index,
							wrapper);
	}

	set_potential_inputs(scope);
}
