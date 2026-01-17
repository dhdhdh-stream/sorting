#include "experiment.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "network.h"
#include "scope.h"

using namespace std;

void boost_try(vector<vector<double>>& train_obs_histories,
			   vector<vector<ScopeHistory*>>& train_stack_traces,
			   vector<double>& train_true_histories,
			   vector<vector<double>>& validation_obs_histories,
			   vector<vector<ScopeHistory*>>& validation_stack_traces,
			   vector<double>& validation_true_histories,
			   Network*& best_true_network,
			   double& best_sum_misguess,
			   int& best_num_positive,
			   double& best_sum_predicted_score) {
	geometric_distribution<int> num_obs_distribution(0.2);
	int num_obs;
	while (true) {
		num_obs = 2 + num_obs_distribution(generator);
		if (num_obs <= (int)train_obs_histories[0].size()) {
			break;
		}
	}

	vector<int> remaining_indexes(train_obs_histories[0].size());
	for (int i_index = 0; i_index < (int)train_obs_histories[0].size(); i_index++) {
		remaining_indexes[i_index] = i_index;
	}

	vector<int> obs_indexes;
	for (int o_index = 0; o_index < num_obs; o_index++) {
		uniform_int_distribution<int> distribution(0, remaining_indexes.size()-1);
		int index = distribution(generator);
		obs_indexes.push_back(remaining_indexes[index]);
		remaining_indexes.erase(remaining_indexes.begin() + index);
	}

	vector<vector<double>> train_inputs(train_stack_traces.size());
	for (int h_index = 0; h_index < (int)train_stack_traces.size(); h_index++) {
		train_inputs[h_index] = vector<double>(num_obs);
		for (int o_index = 0; o_index < num_obs; o_index++) {
			train_inputs[h_index][o_index] = train_stack_traces[h_index].back()->obs_history[obs_indexes[o_index]];
		}
	}

	uniform_int_distribution<int> input_distribution(0, train_obs_histories.size()-1);

	Network* signal_network = new Network(num_obs,
										  NETWORK_SIZE_SMALL);
	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		int rand_index = input_distribution(generator);

		signal_network->activate(train_inputs[rand_index]);

		double error = train_true_histories[rand_index] - signal_network->output->acti_vals[0];

		signal_network->backprop(error);
	}

	vector<double> train_signals(train_inputs.size());
	for (int h_index = 0; h_index < (int)train_inputs.size(); h_index++) {
		signal_network->activate(train_inputs[h_index]);
		train_signals[h_index] = signal_network->output->acti_vals[0];
	}

	delete signal_network;

	Network* true_network = new Network(train_obs_histories[0].size(),
										NETWORK_SIZE_SMALL);
	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		int rand_index = input_distribution(generator);

		true_network->activate(train_obs_histories[rand_index]);

		double error = train_signals[rand_index] - true_network->output->acti_vals[0];

		true_network->backprop(error);
	}

	double sum_misguess = 0.0;
	int num_positive = 0;
	double sum_predicted_score = 0.0;
	for (int h_index = 0; h_index < (int)validation_obs_histories.size(); h_index++) {
		true_network->activate(validation_obs_histories[h_index]);
		double predicted_score = true_network->output->acti_vals[0];
		sum_misguess += (validation_true_histories[h_index] - predicted_score) * (validation_true_histories[h_index] - predicted_score);

		if (predicted_score > 0.0) {
			num_positive++;
		}

		if (predicted_score >= 0.0) {
			sum_predicted_score += validation_true_histories[h_index];
		}
	}

	if (sum_misguess < best_sum_misguess) {
		delete best_true_network;
		best_true_network = true_network;
		best_sum_misguess = sum_misguess;
		best_num_positive = num_positive;
		best_sum_predicted_score = sum_predicted_score;
	} else {
		delete true_network;
	}
}
