#include "decision_tree.h"

#include <iostream>

#include "constants.h"
#include "decision_tree_node.h"
#include "globals.h"
#include "network.h"

using namespace std;

const int NUM_INIT_TRIES = 10;

void DecisionTree::init_helper() {
	vector<int> best_input_indexes;
	Network* best_network = NULL;

	double best_sum_misguess = numeric_limits<double>::max();

	uniform_int_distribution<int> input_distribution(0, this->obs_histories[0].size()-1);
	for (int try_index = 0; try_index < NUM_INIT_TRIES; try_index++) {
		vector<int> remaining_indexes(this->obs_histories[0].size());
		for (int i_index = 0; i_index < (int)this->obs_histories[0].size(); i_index++) {
			remaining_indexes[i_index] = i_index;
		}

		vector<int> curr_input_indexes;
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

		Network* curr_network = new Network(1 + curr_input_indexes.size());

		uniform_int_distribution<int> distribution(0, DT_NUM_TRAIN_SAMPLES-1);
		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
			int index = distribution(generator);

			vector<double> inputs(1 + curr_input_indexes.size());
			inputs[0] = 0.0;
			for (int i_index = 0; i_index < (int)curr_input_indexes.size(); i_index++) {
				inputs[1 + i_index] = this->obs_histories[index][curr_input_indexes[i_index]];
			}

			curr_network->activate(inputs);

			double error = this->target_val_histories[index] - curr_network->output->acti_vals[0];

			curr_network->backprop(error);
		}

		double curr_sum_misguess = 0.0;
		for (int h_index = 0; h_index < DT_NUM_TEST_SAMPLES; h_index++) {
			vector<double> inputs(1 + curr_input_indexes.size());
			inputs[0] = 0.0;
			for (int i_index = 0; i_index < (int)curr_input_indexes.size(); i_index++) {
				inputs[i_index] = this->obs_histories[DT_NUM_TRAIN_SAMPLES + h_index][curr_input_indexes[i_index]];
			}

			curr_network->activate(inputs);

			curr_sum_misguess += (this->target_val_histories[DT_NUM_TRAIN_SAMPLES + h_index] - curr_network->output->acti_vals[0])
				* (this->target_val_histories[DT_NUM_TRAIN_SAMPLES + h_index] - curr_network->output->acti_vals[0]);
		}

		if (curr_sum_misguess < best_sum_misguess) {
			best_input_indexes = curr_input_indexes;
			if (best_network != NULL) {
				delete best_network;
			}
			best_network = curr_network;

			best_sum_misguess = curr_sum_misguess;
		} else {
			delete curr_network;
		}
	}

	DecisionTreeNode* new_node = new DecisionTreeNode();
	new_node->id = this->node_counter;
	this->node_counter++;
	this->nodes[new_node->id] = new_node;

	new_node->is_previous = false;
	new_node->input_indexes = best_input_indexes;
	new_node->network = best_network;

	new_node->has_split = false;
	new_node->obs_index = -1;
	new_node->rel_obs_index = -1;
	new_node->split_type = -1;
	new_node->split_target = 0.0;
	new_node->split_range = 0.0;

	new_node->original_node_id = -1;
	new_node->original_node = NULL;
	new_node->branch_node_id = -1;
	new_node->branch_node = NULL;

	this->root = new_node;

	measure_helper();
}
