#include "decision_tree_helpers.h"

#include "constants.h"
#include "decision_tree.h"
#include "globals.h"
#include "network.h"

using namespace std;

void network_try(vector<vector<double>>& train_obs_histories,
				 vector<double>& train_previous_val_histories,
				 vector<double>& train_target_val_histories,
				 vector<int>& curr_input_indexes,
				 Network*& curr_network,
				 vector<vector<double>>& test_obs_histories,
				 vector<double>& test_previous_val_histories,
				 vector<double>& test_target_val_histories,
				 double& curr_sum_misguess) {
	vector<int> remaining_indexes(train_obs_histories[0].size());
	for (int i_index = 0; i_index < (int)train_obs_histories[0].size(); i_index++) {
		remaining_indexes[i_index] = i_index;
	}

	while (true) {
		if (remaining_indexes.size() == 0) {
			break;
		}

		uniform_int_distribution<int> distribution(0, remaining_indexes.size()-1);
		int index = distribution(generator);

		curr_input_indexes.push_back(remaining_indexes[index]);

		remaining_indexes.erase(remaining_indexes.begin() + index);

		if (curr_input_indexes.size() >= DT_NODE_MAX_NUM_INPUTS) {
			break;
		}
	}

	curr_network = new Network(1 + curr_input_indexes.size());

	uniform_int_distribution<int> match_distribution(0, train_obs_histories.size()-1);
	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		int index = match_distribution(generator);

		vector<double> inputs(1 + curr_input_indexes.size());
		inputs[0] = train_previous_val_histories[index];
		for (int i_index = 0; i_index < (int)curr_input_indexes.size(); i_index++) {
			inputs[1 + i_index] = train_obs_histories[index][curr_input_indexes[i_index]];
		}

		curr_network->activate(inputs);

		double error = train_target_val_histories[index] - curr_network->output->acti_vals[0];

		curr_network->backprop(error);
	}

	curr_sum_misguess = 0.0;
	for (int h_index = 0; h_index < (int)test_obs_histories.size(); h_index++) {
		vector<double> inputs(1 + curr_input_indexes.size());
		inputs[0] = test_previous_val_histories[h_index];
		for (int i_index = 0; i_index < (int)curr_input_indexes.size(); i_index++) {
			inputs[1 + i_index] = train_obs_histories[h_index][curr_input_indexes[i_index]];
		}

		curr_network->activate(inputs);

		curr_sum_misguess += (test_target_val_histories[h_index] - curr_network->output->acti_vals[0])
			* (test_target_val_histories[h_index] - curr_network->output->acti_vals[0]);
	}
}
