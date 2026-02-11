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
const int INIT_ITERS = 15;
const int REFINE_ITERS = 15;
#else
const int INIT_ITERS = 150000;
const int REFINE_ITERS = 150000;
#endif /* MDEBUG */

const double VALIDATION_RATIO = 0.2;

void update_network(vector<vector<double>>& obs_histories,
					vector<double>& target_val_histories,
					BuildNetwork*& network) {
	int num_train_samples = (1.0 - VALIDATION_RATIO) * (int)obs_histories.size();

	if (network->nodes.size() == 0) {
		double sum_vals = 0.0;
		for (int h_index = 0; h_index < num_train_samples; h_index++) {
			sum_vals += target_val_histories[h_index];
		}
		network->output_constant = sum_vals / num_train_samples;
	}

	vector<vector<double>> node_vals(obs_histories.size());
	vector<double> remaining_vals(obs_histories.size());
	for (int h_index = 0; h_index < (int)obs_histories.size(); h_index++) {
		double val = network->activate(obs_histories[h_index]);

		vector<double> curr_node_vals(network->nodes.size());
		for (int n_index = 0; n_index < (int)network->nodes.size(); n_index++) {
			curr_node_vals[n_index] = network->nodes[n_index]->output->acti_vals[0];
		}
		node_vals[h_index] = curr_node_vals;

		remaining_vals[h_index] = target_val_histories[h_index] - val;
	}

	double existing_sum_misguess = 0.0;
	for (int h_index = num_train_samples; h_index < (int)obs_histories.size(); h_index++) {
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

		uniform_int_distribution<int> train_distribution(0, num_train_samples-1);

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

			curr_node->init_activate(obs_histories[index],
									 node_vals[index]);

			double error = remaining_vals[index] - curr_node->output->acti_vals[0];
			curr_node->output->errors[0] = error;
			curr_node->init_backprop();

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

		BuildNetwork* curr_network = new BuildNetwork();
		curr_network->copy_from(network);

		if (curr_network->nodes.size() == 0) {
			curr_network->inputs = vector<double>(obs_histories[0].size());
		}

		curr_network->nodes.push_back(curr_node);

		curr_network->output_weights.push_back(1.0);
		curr_network->output_weight_updates.push_back(0.0);

		for (int iter_index = 0; iter_index < REFINE_ITERS; iter_index++) {
			int index = train_distribution(generator);
			curr_network->backprop(obs_histories[index],
								   target_val_histories[index]);
		}

		double curr_sum_misguess = 0.0;
		for (int h_index = num_train_samples; h_index < (int)obs_histories.size(); h_index++) {
			double val = curr_network->activate(obs_histories[h_index]);

			curr_sum_misguess += (target_val_histories[h_index] - val)
				* (target_val_histories[h_index] - val);
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
		delete network;
		network = best_network;
	}
}
