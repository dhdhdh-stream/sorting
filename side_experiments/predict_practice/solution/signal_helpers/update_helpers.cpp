#include "signal_helpers.h"

#include <algorithm>
#include <iostream>

#include "constants.h"
#include "globals.h"
#include "layer.h"
#include "signal.h"
#include "signal_node.h"
#include "solution.h"
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

const int MAX_NUM_FAIL = 2;

void clean_pre_signal_helper(double& existing_sum_misguess,
							 Solution* solution,
							 int node_index,
							 SolutionWrapper* wrapper) {
	Signal* curr_signal = new Signal();
	curr_signal->copy_from(solution->signal);

	delete curr_signal->nodes[node_index];
	curr_signal->nodes.erase(curr_signal->nodes.begin() + node_index);

	curr_signal->output_weights.erase(curr_signal->output_weights.begin() + node_index);
	curr_signal->output_weight_updates.erase(curr_signal->output_weight_updates.begin() + node_index);

	for (int h_index = 0; h_index < SIGNAL_NUM_SAMPLES; h_index++) {
		curr_signal->input_val_histories[h_index].erase(
			curr_signal->input_val_histories[h_index].begin() + node_index);
		curr_signal->input_is_on_histories[h_index].erase(
			curr_signal->input_is_on_histories[h_index].begin() + node_index);
	}

	int num_train_samples = (1.0 - VALIDATION_RATIO) * SIGNAL_NUM_SAMPLES;

	uniform_int_distribution<int> train_distribution(0, SIGNAL_NUM_SAMPLES-1);
	for (int iter_index = 0; iter_index < CLEAN_ITERS; iter_index++) {
		int index = train_distribution(generator);
		curr_signal->backprop_helper(index);
	}

	// // temp
	// double train_curr_sum_misguess = 0.0;
	// for (int h_index = 0; h_index < num_train_samples; h_index++) {
	// 	double val = curr_signal->activate_helper(h_index);
	// 	train_curr_sum_misguess += (curr_signal->target_val_histories[h_index] - val)
	// 		* (curr_signal->target_val_histories[h_index] - val);
	// }
	// cout << "train_curr_sum_misguess: " << train_curr_sum_misguess << endl;

	vector<double> predicted_scores;

	double existing_curr_sum_misguess = 0.0;
	for (int h_index = num_train_samples; h_index < SIGNAL_NUM_SAMPLES; h_index++) {
		double val = curr_signal->activate_helper(h_index);

		predicted_scores.push_back(val);

		existing_curr_sum_misguess += (curr_signal->target_val_histories[h_index] - val)
			* (curr_signal->target_val_histories[h_index] - val);
	}

	double curr_sum_misguess = existing_curr_sum_misguess;

	// // temp
	// cout << "clear " << node_index << " " << curr_sum_misguess << endl;

	if (curr_sum_misguess < existing_sum_misguess) {
		existing_sum_misguess = curr_sum_misguess;

		delete solution->signal;
		solution->signal = curr_signal;
	} else {
		delete curr_signal;
	}
}

void update_signal(Solution* solution,
				   SolutionWrapper* wrapper) {
	// temp
	cout << "update_pre_signal" << endl;

	Signal* signal = solution->signal;

	{
		default_random_engine generator_copy = generator;
		shuffle(signal->input_val_histories.begin(), signal->input_val_histories.end(), generator_copy);
	}
	{
		default_random_engine generator_copy = generator;
		shuffle(signal->input_is_on_histories.begin(), signal->input_is_on_histories.end(), generator_copy);
	}
	{
		default_random_engine generator_copy = generator;
		shuffle(signal->target_val_histories.begin(), signal->target_val_histories.end(), generator_copy);
	}
	{
		default_random_engine generator_copy = generator;
		shuffle(signal->potential_input_val_histories.begin(), signal->potential_input_val_histories.end(), generator_copy);
	}
	{
		default_random_engine generator_copy = generator;
		shuffle(signal->potential_input_is_on_histories.begin(), signal->potential_input_is_on_histories.end(), generator_copy);
	}

	// // temp
	// for (int n_index = 0; n_index < (int)signal->nodes.size(); n_index++) {
	// 	cout << "signal->input_val_histories[0][n_index]:" << endl;
	// 	for (int i_index = 0; i_index < (int)signal->nodes[n_index]->inputs.size(); i_index++) {
	// 		cout << signal->input_val_histories[0][n_index][i_index] << " ";
	// 	}
	// 	cout << endl;
	// 	cout << "signal->input_is_on_histories[0][n_index]:" << endl;
	// 	for (int i_index = 0; i_index < (int)signal->nodes[n_index]->inputs.size(); i_index++) {
	// 		cout << signal->input_is_on_histories[0][n_index][i_index] << " ";
	// 	}
	// 	cout << endl;
	// }
	// cout << "signal->target_val_histories[0]: " << signal->target_val_histories[0] << endl;
	// for (int p_index = 0; p_index < (int)signal->potential_inputs.size(); p_index++) {
	// 	cout << "signal->potential_input_val_histories[0][p_index]:" << endl;
	// 	for (int i_index = 0; i_index < (int)signal->potential_inputs[p_index].size(); i_index++) {
	// 		cout << signal->potential_input_val_histories[0][p_index][i_index] << " ";
	// 	}
	// 	cout << endl;
	// 	cout << "signal->potential_input_is_on_histories[0][n_index]:" << endl;
	// 	for (int i_index = 0; i_index < (int)signal->potential_inputs[p_index].size(); i_index++) {
	// 		cout << signal->potential_input_is_on_histories[0][p_index][i_index] << " ";
	// 	}
	// 	cout << endl;
	// }

	int num_train_samples = (1.0 - VALIDATION_RATIO) * SIGNAL_NUM_SAMPLES;

	vector<double> remaining_vals(SIGNAL_NUM_SAMPLES);
	for (int h_index = 0; h_index < SIGNAL_NUM_SAMPLES; h_index++) {
		double val = signal->activate_helper(h_index);
		remaining_vals[h_index] = signal->target_val_histories[h_index] - val;
	}

	// // temp
	// double train_sum_misguess = 0.0;
	// for (int h_index = 0; h_index < num_train_samples; h_index++) {
	// 	train_sum_misguess += remaining_vals[h_index] * remaining_vals[h_index];
	// }
	// cout << "train_sum_misguess: " << train_sum_misguess << endl;

	double existing_sum_misguess = 0.0;
	for (int h_index = num_train_samples; h_index < SIGNAL_NUM_SAMPLES; h_index++) {
		existing_sum_misguess += remaining_vals[h_index] * remaining_vals[h_index];
	}

	// // temp
	// cout << "existing_sum_misguess: " << existing_sum_misguess << endl;

	Signal* best_signal = NULL;
	double best_sum_misguess = numeric_limits<double>::max();

	uniform_int_distribution<int> train_distribution(0, num_train_samples-1);
	for (int try_index = 0; try_index < (int)signal->potential_inputs.size(); try_index++) {
		vector<double> input_averages;
		vector<double> input_standard_deviations;
		for (int i_index = 0; i_index < (int)signal->potential_inputs[try_index].size(); i_index++) {
			double sum_vals = 0.0;
			int count = 0;
			for (int h_index = 0; h_index < num_train_samples; h_index++) {
				if (signal->potential_input_is_on_histories[h_index][try_index][i_index]) {
					sum_vals += signal->potential_input_val_histories[h_index][try_index][i_index];
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
					if (signal->potential_input_is_on_histories[h_index][try_index][i_index]) {
						double val = signal->potential_input_val_histories[h_index][try_index][i_index];
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
			int index = train_distribution(generator);

			curr_node->activate(signal->potential_input_val_histories[index][try_index],
								signal->potential_input_is_on_histories[index][try_index]);

			double error = remaining_vals[index] - curr_node->output->acti_vals[0];
			curr_node->output->errors[0] = error;
			curr_node->backprop();

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
		curr_signal->copy_from(signal);

		curr_signal->nodes.push_back(curr_node);

		curr_signal->output_weights.push_back(1.0);
		curr_signal->output_weight_updates.push_back(0.0);

		for (int h_index = 0; h_index < SIGNAL_NUM_SAMPLES; h_index++) {
			curr_signal->input_val_histories[h_index]
				.push_back(curr_signal->potential_input_val_histories[h_index][try_index]);
			curr_signal->input_is_on_histories[h_index]
				.push_back(curr_signal->potential_input_is_on_histories[h_index][try_index]);
		}

		curr_signal->potential_inputs.clear();

		for (int iter_index = 0; iter_index < REFINE_ITERS; iter_index++) {
			int index = train_distribution(generator);
			curr_signal->backprop_helper(index);
		}

		// // temp
		// double train_curr_sum_misguess = 0.0;
		// for (int h_index = 0; h_index < num_train_samples; h_index++) {
		// 	double val = curr_signal->activate_helper(h_index);
		// 	train_curr_sum_misguess += (curr_signal->target_val_histories[h_index] - val)
		// 		* (curr_signal->target_val_histories[h_index] - val);
		// }
		// cout << "train_curr_sum_misguess: " << train_curr_sum_misguess << endl;

		vector<double> predicted_scores;

		double curr_sum_misguess = 0.0;
		for (int h_index = num_train_samples; h_index < SIGNAL_NUM_SAMPLES; h_index++) {
			double val = curr_signal->activate_helper(h_index);

			predicted_scores.push_back(val);

			curr_sum_misguess += (curr_signal->target_val_histories[h_index] - val)
				* (curr_signal->target_val_histories[h_index] - val);
		}

		// cout << "curr_sum_misguess: " << curr_sum_misguess << endl;

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

		delete solution->signal;
		solution->signal = best_signal;
	}

	solution->signal->potential_inputs.clear();

	for (int n_index = (int)solution->signal->nodes.size()-1; n_index >= 0; n_index--) {
		clean_pre_signal_helper(existing_sum_misguess,
								solution,
								n_index,
								wrapper);
	}

	set_signal_potential_inputs(solution);
}
