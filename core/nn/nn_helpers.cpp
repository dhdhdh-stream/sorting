#include "nn_helpers.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "network.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_ITERS_FRONT = 20;
const int TRAIN_ITERS_BACK = 10;
const int OPTIMIZE_ITERS = 30;
#else
const int TRAIN_ITERS_FRONT = 200000;
const int TRAIN_ITERS_BACK = 100000;
const int OPTIMIZE_ITERS = 100000;
#endif /* MDEBUG */

const double NETWORK_INPUT_MIN_IMPACT = 0.2;

void train_network(vector<vector<vector<double>>>& inputs,
				   vector<double>& target_vals,
				   vector<vector<Scope*>>& test_input_scope_contexts,
				   vector<vector<AbstractNode*>>& test_input_node_contexts,
				   vector<int>& test_input_obs_indexes,
				   Network* network) {
	int train_instances = (1.0 - TEST_SAMPLES_PERCENTAGE) * inputs.size();

	int num_new_inputs = (int)network->inputs.back()->acti_vals.size();

	uniform_int_distribution<int> distribution(0, train_instances-1);
	for (int iter_index = 0; iter_index < TRAIN_ITERS_FRONT; iter_index++) {
		int rand_index = distribution(generator);

		network->activate(inputs[rand_index]);

		double error = target_vals[rand_index] - network->output->acti_vals[0];

		network->backprop(error);
	}

	/**
	 * TODO: compare against strongest signal overall instead of new
	 */

	vector<double> input_means(num_new_inputs);
	for (int i_index = 0; i_index < num_new_inputs; i_index++) {
		double sum_vals = 0.0;
		for (int d_index = 0; d_index < train_instances; d_index++) {
			sum_vals += inputs[d_index].back()[i_index];
		}
		input_means[i_index] = sum_vals / train_instances;
	}

	vector<double> input_weights(num_new_inputs);
	for (int i_index = 0; i_index < num_new_inputs; i_index++) {
		double sum_vals = 0.0;
		for (int d_index = 0; d_index < train_instances; d_index++) {
			sum_vals += abs(inputs[d_index].back()[i_index] - input_means[i_index]);
		}
		input_weights[i_index] = sum_vals / train_instances;
	}

	vector<double> input_impacts(num_new_inputs);
	for (int i_index = 0; i_index < num_new_inputs; i_index++) {
		double sum_impact = 0.0;
		for (int n_index = 0; n_index < NETWORK_INCREMENT_HIDDEN_SIZE; n_index++) {
			/**
			 * - if NETWORK_INCREMENT_TYPE_SIDE, then only 1 layer
			 * - if NETWORK_INCREMENT_TYPE_ABOVE, then last input layer is new
			 */
			sum_impact += abs(network->hiddens.back()->weights[n_index].back()[i_index]);
		}
		input_impacts[i_index] = sum_impact * input_weights[i_index];
	}

	double max_impact = 0.0;
	for (int i_index = 0; i_index < num_new_inputs; i_index++) {
		if (input_impacts[i_index] > max_impact) {
			max_impact = input_impacts[i_index];
		}
	}

	for (int i_index = num_new_inputs-1; i_index >= 0; i_index--) {
		#if defined(MDEBUG) && MDEBUG
		if (rand()%2 == 0) {
		#else
		if (input_impacts[i_index] < max_impact * NETWORK_INPUT_MIN_IMPACT) {
		#endif /* MDEBUG */
			test_input_scope_contexts.erase(test_input_scope_contexts.begin() + i_index);
			test_input_node_contexts.erase(test_input_node_contexts.begin() + i_index);
			test_input_obs_indexes.erase(test_input_obs_indexes.begin() + i_index);

			network->inputs.back()->acti_vals.erase(
				network->inputs.back()->acti_vals.begin() + i_index);
			network->inputs.back()->errors.erase(
				network->inputs.back()->errors.begin() + i_index);

			for (int n_index = 0; n_index < NETWORK_INCREMENT_HIDDEN_SIZE; n_index++) {
				network->hiddens.back()->weights[n_index].back().erase(
					network->hiddens.back()->weights[n_index].back().begin() + i_index);
				network->hiddens.back()->weight_updates[n_index].back().erase(
					network->hiddens.back()->weight_updates[n_index].back().begin() + i_index);
			}

			for (int d_index = 0; d_index < (int)inputs.size(); d_index++) {
				inputs[d_index].back().erase(inputs[d_index].back().begin() + i_index);
			}
		}
	}

	for (int iter_index = 0; iter_index < TRAIN_ITERS_BACK; iter_index++) {
		int rand_index = distribution(generator);

		network->activate(inputs[rand_index]);

		double error = target_vals[rand_index] - network->output->acti_vals[0];

		network->backprop(error);
	}
}

void measure_network(vector<vector<vector<double>>>& inputs,
					 vector<double>& target_vals,
					 Network* network,
					 double& average_misguess,
					 double& misguess_standard_deviation) {
	int train_instances = (1.0 - TEST_SAMPLES_PERCENTAGE) * inputs.size();
	int test_instances = inputs.size() - train_instances;

	vector<double> predicted_outputs(test_instances);
	for (int d_index = 0; d_index < test_instances; d_index++) {
		network->activate(inputs[train_instances + d_index]);
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

void optimize_network(vector<vector<vector<double>>>& inputs,
					  vector<double>& target_vals,
					  Network* network) {
	int train_instances = (1.0 - TEST_SAMPLES_PERCENTAGE) * inputs.size();

	uniform_int_distribution<int> distribution(0, train_instances-1);
	for (int iter_index = 0; iter_index < TRAIN_ITERS_FRONT; iter_index++) {
		int rand_index = distribution(generator);

		network->activate(inputs[rand_index]);

		double error = target_vals[rand_index] - network->output->acti_vals[0];

		network->backprop(error);
	}
}
