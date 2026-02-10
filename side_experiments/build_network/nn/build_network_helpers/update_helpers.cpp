#include "build_network.h"

#include <iostream>

#include "build_node.h"
#include "constants.h"
#include "globals.h"
#include "layer.h"

using namespace std;

const int NUM_TRIES = 10;

const double INIT_NETWORK_TARGET_MAX_UPDATE = 0.01;
const int INIT_EPOCH_SIZE = 20;

void BuildNetwork::update_helper() {
	vector<vector<double>> node_vals(this->obs_histories.size());
	vector<double> remaining_vals(this->obs_histories.size());
	for (int h_index = 0; h_index < (int)this->obs_histories.size(); h_index++) {
		double val = activate(this->obs_histories[h_index]);

		vector<double> curr_node_vals(this->nodes.size());
		for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
			curr_node_vals[n_index] = this->nodes[n_index]->output->acti_vals[0];
		}
		node_vals[h_index] = curr_node_vals;

		remaining_vals[h_index] = this->target_val_histories[h_index] - val;
	}

	double existing_sum_misguess = 0.0;
	for (int h_index = BUILD_NUM_TRAIN_SAMPLES; h_index < BUILD_NUM_TOTAL_SAMPLES; h_index++) {
		existing_sum_misguess += remaining_vals[h_index] * remaining_vals[h_index];
	}

	BuildNode* best_node = NULL;
	double best_improvement = 0.0;

	for (int try_index = 0; try_index < 10; try_index++) {
		vector<pair<int,int>> possible_inputs;
		for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
			possible_inputs.push_back({INPUT_TYPE_INPUT, i_index});
		}
		for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
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

		int epoch_iter = 0;
		double average_max_update = 0.0;
		uniform_int_distribution<int> train_distribution(0, BUILD_NUM_TRAIN_SAMPLES-1);
		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
			int index = train_distribution(generator);

			curr_node->init_activate(this->obs_histories[index],
									 node_vals[index]);

			double error = remaining_vals[index] - curr_node->output->acti_vals[0];
			curr_node->output->errors[0] = error;
			curr_node->init_backprop();

			epoch_iter++;
			if (epoch_iter == INIT_EPOCH_SIZE) {
				double max_update = 0.0;
				curr_node->get_max_update(max_update);

				average_max_update = 0.999*average_max_update + 0.001*max_update;
				if (max_update > 0.0) {
					double learning_rate = (0.3*INIT_NETWORK_TARGET_MAX_UPDATE)/average_max_update;
					if (learning_rate * max_update > INIT_NETWORK_TARGET_MAX_UPDATE) {
						learning_rate = INIT_NETWORK_TARGET_MAX_UPDATE/max_update;
					}

					curr_node->update_weights(learning_rate);
				}

				epoch_iter = 0;
			}
		}

		double curr_sum_misguess = 0.0;
		for (int h_index = BUILD_NUM_TRAIN_SAMPLES; h_index < BUILD_NUM_TOTAL_SAMPLES; h_index++) {
			curr_node->init_activate(this->obs_histories[h_index],
									 node_vals[h_index]);

			curr_sum_misguess += (remaining_vals[h_index] - curr_node->output->acti_vals[0])
				* (remaining_vals[h_index] - curr_node->output->acti_vals[0]);
		}

		double curr_improvement = existing_sum_misguess - curr_sum_misguess;
		if (curr_improvement > best_improvement) {
			if (best_node != NULL) {
				delete best_node;
			}
			best_node = curr_node;
			best_improvement = curr_improvement;
		} else {
			delete curr_node;
		}
	}

	if (best_improvement > 0.0) {
		this->nodes.push_back(best_node);

		this->output_weights.push_back(1.0);
		this->output_weight_updates.push_back(0.0);
	}
}
