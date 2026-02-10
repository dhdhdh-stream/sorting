#include "build_network_helpers.h"

#include <iostream>

#include "build_network.h"
#include "build_node.h"
#include "constants.h"
#include "globals.h"
#include "layer.h"

using namespace std;

const int NUM_TRIES = 10;

#if defined(MDEBUG) && MDEBUG
const int INIT_ITERS = 10;
const int REFINE_ITERS = 20;
#else
const int INIT_ITERS = 100000;
const int REFINE_ITERS = 200000;
#endif /* MDEBUG */

void update_network(BuildNetwork*& network) {
	vector<vector<double>> node_vals(network->obs_histories.size());
	vector<double> remaining_vals(network->obs_histories.size());
	for (int h_index = 0; h_index < (int)network->obs_histories.size(); h_index++) {
		double val = network->activate(network->obs_histories[h_index]);

		vector<double> curr_node_vals(network->nodes.size());
		for (int n_index = 0; n_index < (int)network->nodes.size(); n_index++) {
			curr_node_vals[n_index] = network->nodes[n_index]->output->acti_vals[0];
		}
		node_vals[h_index] = curr_node_vals;

		remaining_vals[h_index] = network->target_val_histories[h_index] - val;
	}

	double existing_sum_misguess = 0.0;
	for (int h_index = BUILD_NUM_TRAIN_SAMPLES; h_index < BUILD_NUM_TOTAL_SAMPLES; h_index++) {
		existing_sum_misguess += remaining_vals[h_index] * remaining_vals[h_index];
	}

	BuildNetwork* best_network = NULL;
	double best_improvement = 0.0;

	for (int try_index = 0; try_index < 10; try_index++) {
		vector<pair<int,int>> possible_inputs;
		for (int i_index = 0; i_index < (int)network->inputs.size(); i_index++) {
			possible_inputs.push_back({INPUT_TYPE_INPUT, i_index});
		}
		for (int n_index = 0; n_index < (int)network->nodes.size(); n_index++) {
			possible_inputs.push_back({INPUT_TYPE_NODE, n_index});
		}

		vector<int> input_types;
		vector<int> input_indexes;
		while (true) {
			if (possible_inputs.size() == 0) {
				break;
			}

			uniform_int_distribution<int> distribution(0, possible_inputs.size()-1);
			int index = distribution(generator);

			input_types.push_back(possible_inputs[index].first);
			input_indexes.push_back(possible_inputs[index].second);

			possible_inputs.erase(possible_inputs.begin() + index);

			if (input_types.size() >= BUILD_MAX_NUM_INPUTS) {
				break;
			}
		}
		BuildNode* curr_node = new BuildNode(input_types,
											 input_indexes);

		uniform_int_distribution<int> train_distribution(0, BUILD_NUM_TRAIN_SAMPLES-1);

		for (int iter_index = 0; iter_index < INIT_ITERS; iter_index++) {
			int index = train_distribution(generator);

			curr_node->init_activate(network->obs_histories[index],
									 node_vals[index]);

			double error = remaining_vals[index] - curr_node->output->acti_vals[0];
			curr_node->output->errors[0] = error;
			curr_node->init_backprop();
		}

		BuildNetwork* curr_network = new BuildNetwork(network);

		curr_network->nodes.push_back(curr_node);

		curr_network->output_weights.push_back(1.0);
		curr_network->output_weight_updates.push_back(0.0);

		for (int iter_index = 0; iter_index < REFINE_ITERS; iter_index++) {
			int index = train_distribution(generator);
			curr_network->backprop_iter_helper(index);
		}

		double curr_sum_misguess = 0.0;
		for (int h_index = BUILD_NUM_TRAIN_SAMPLES; h_index < BUILD_NUM_TOTAL_SAMPLES; h_index++) {
			double val = curr_network->activate(network->obs_histories[h_index]);

			curr_sum_misguess += (network->target_val_histories[h_index] - val)
				* (network->target_val_histories[h_index] - val);
		}

		double curr_improvement = existing_sum_misguess - curr_sum_misguess;
		if (curr_improvement > best_improvement) {
			if (best_network != NULL) {
				delete best_network;
			}
			best_network = curr_network;
			best_improvement = curr_improvement;
		} else {
			delete curr_network;
		}
	}

	if (best_improvement > 0.0) {
		network = best_network;
	}
}
