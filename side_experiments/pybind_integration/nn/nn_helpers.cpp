#include "nn_helpers.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "network.h"

using namespace std;

const int TRAIN_ITERS = 300000;
const int OPTIMIZE_ITERS = 100000;

void train_network(vector<vector<double>>& inputs,
				   vector<vector<bool>>& input_is_on,
				   vector<double>& target_vals,
				   Network* network) {
	int train_instances = (1.0 - TEST_SAMPLES_PERCENTAGE) * inputs.size();
	if (train_instances > 0) {
		uniform_int_distribution<int> distribution(0, train_instances-1);
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
	int train_instances = (1.0 - TEST_SAMPLES_PERCENTAGE) * inputs.size();
	int test_instances = inputs.size() - train_instances;

	vector<double> predicted_outputs(test_instances);
	for (int d_index = 0; d_index < test_instances; d_index++) {
		network->activate(inputs[train_instances + d_index],
						  input_is_on[train_instances + d_index]);
		predicted_outputs[d_index] = network->output->acti_vals[0];
	}

	double sum_misguess = 0.0;
	for (int d_index = 0; d_index < test_instances; d_index++) {
		sum_misguess += (target_vals[train_instances + d_index] - predicted_outputs[d_index]) * (target_vals[train_instances + d_index] - predicted_outputs[d_index]);
	}
	average_misguess = sum_misguess / test_instances;

	double sum_misguess_variance = 0.0;
	for (int d_index = 0; d_index < test_instances; d_index++) {
		double curr_misguess = (target_vals[train_instances + d_index] - predicted_outputs[d_index]) * (target_vals[train_instances + d_index] - predicted_outputs[d_index]);
		sum_misguess_variance += (curr_misguess - average_misguess) * (curr_misguess - average_misguess);
	}
	misguess_standard_deviation = sqrt(sum_misguess_variance / test_instances);
	if (misguess_standard_deviation < MIN_STANDARD_DEVIATION) {
		misguess_standard_deviation = MIN_STANDARD_DEVIATION;
	}
}

void optimize_network(vector<vector<double>>& inputs,
					  vector<vector<bool>>& input_is_on,
					  vector<double>& target_vals,
					  Network* network) {
	int train_instances = (1.0 - TEST_SAMPLES_PERCENTAGE) * inputs.size();
	if (train_instances > 0) {
		uniform_int_distribution<int> distribution(0, train_instances-1);
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
