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
const int INIT_ITERS = 20;
const int REFINE_ITERS = 10;
const int CLEAN_ITERS = 10;
#else
const int INIT_ITERS = 200000;
const int REFINE_ITERS = 100000;
const int CLEAN_ITERS = 100000;
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

	vector<double> remaining_vals(obs_histories.size());
	for (int h_index = 0; h_index < (int)obs_histories.size(); h_index++) {
		double val = network->activate(obs_histories[h_index]);
		remaining_vals[h_index] = target_val_histories[h_index] - val;
	}

	double existing_sum_misguess = 0.0;
	for (int h_index = num_train_samples; h_index < (int)obs_histories.size(); h_index++) {
		existing_sum_misguess += remaining_vals[h_index] * remaining_vals[h_index];
	}

	BuildNetwork* best_network = NULL;
	double best_improvement = 0.0;

	for (int try_index = 0; try_index < 10; try_index++) {
		vector<int> possible_inputs;
		for (int i_index = 0; i_index < (int)obs_histories[0].size(); i_index++) {
			possible_inputs.push_back(i_index);
		}

		vector<int> input_indexes;
		while (true) {
			if (possible_inputs.size() == 0) {
				break;
			}

			uniform_int_distribution<int> distribution(0, possible_inputs.size()-1);
			int index = distribution(generator);

			input_indexes.push_back(possible_inputs[index]);

			possible_inputs.erase(possible_inputs.begin() + index);

			if (input_indexes.size() >= BUILD_MAX_NUM_INPUTS) {
				break;
			}
		}
		BuildNode* curr_node = new BuildNode(input_indexes);

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

			curr_node->activate(obs_histories[index]);

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

void clean_network_helper(vector<vector<double>>& existing_obs_histories,
						  vector<double>& existing_target_val_histories,
						  vector<vector<double>>& explore_obs_histories,
						  vector<double>& explore_target_val_histories,
						  double& existing_sum_misguess,
						  BuildNetwork*& network,
						  int node_index) {
	BuildNetwork* curr_network = new BuildNetwork();
	curr_network->copy_from(network);

	delete curr_network->nodes[node_index];
	curr_network->nodes.erase(curr_network->nodes.begin() + node_index);

	curr_network->output_weights.erase(curr_network->output_weights.begin() + node_index);
	curr_network->output_weight_updates.erase(curr_network->output_weight_updates.begin() + node_index);

	int num_train_samples = (1.0 - VALIDATION_RATIO) * (int)explore_obs_histories.size();

	uniform_int_distribution<int> existing_distribution(0, 1);
	uniform_int_distribution<int> train_distribution(0, num_train_samples-1);
	for (int iter_index = 0; iter_index < CLEAN_ITERS; iter_index++) {
		if (existing_distribution(generator)) {
			int index = train_distribution(generator);
			curr_network->backprop(existing_obs_histories[index],
								   existing_target_val_histories[index]);
		} else {
			int index = train_distribution(generator);
			curr_network->backprop(explore_obs_histories[index],
								   explore_target_val_histories[index]);
		}
	}

	double existing_curr_sum_misguess = 0.0;
	for (int h_index = num_train_samples; h_index < (int)existing_obs_histories.size(); h_index++) {
		double val = curr_network->activate(existing_obs_histories[h_index]);

		existing_curr_sum_misguess += (existing_target_val_histories[h_index] - val)
			* (existing_target_val_histories[h_index] - val);
	}

	double explore_curr_sum_misguess = 0.0;
	for (int h_index = num_train_samples; h_index < (int)explore_obs_histories.size(); h_index++) {
		double val = curr_network->activate(explore_obs_histories[h_index]);

		explore_curr_sum_misguess += (explore_target_val_histories[h_index] - val)
			* (explore_target_val_histories[h_index] - val);
	}

	double curr_sum_misguess = (existing_curr_sum_misguess + explore_curr_sum_misguess) / 2.0;

	if (curr_sum_misguess < existing_sum_misguess) {
		existing_sum_misguess = curr_sum_misguess;

		delete network;
		network = curr_network;
	} else {
		delete curr_network;
	}
}

/**
 * - weigh existing vs. explore 1-to-1
 *   - existing may have high variance
 *   - only rely on explore to generalize slightly off of existing
 */
void update_network(vector<vector<double>>& existing_obs_histories,
					vector<double>& existing_target_val_histories,
					vector<vector<double>>& explore_obs_histories,
					vector<double>& explore_target_val_histories,
					BuildNetwork*& network) {
	int num_train_samples = (1.0 - VALIDATION_RATIO) * (int)explore_obs_histories.size();

	if (network->nodes.size() == 0) {
		double existing_sum_vals = 0.0;
		for (int h_index = 0; h_index < num_train_samples; h_index++) {
			existing_sum_vals += existing_target_val_histories[h_index];
		}
		double existing_val_average = existing_sum_vals / num_train_samples;

		double explore_sum_vals = 0.0;
		for (int h_index = 0; h_index < num_train_samples; h_index++) {
			explore_sum_vals += explore_target_val_histories[h_index];
		}
		double explore_val_average = explore_sum_vals / num_train_samples;

		network->output_constant = (existing_val_average + explore_val_average) / 2.0;
	}

	vector<double> existing_remaining_vals(existing_obs_histories.size());
	for (int h_index = 0; h_index < (int)existing_obs_histories.size(); h_index++) {
		double val = network->activate(existing_obs_histories[h_index]);
		existing_remaining_vals[h_index] = existing_target_val_histories[h_index] - val;
	}

	vector<double> explore_remaining_vals(explore_obs_histories.size());
	for (int h_index = 0; h_index < (int)explore_obs_histories.size(); h_index++) {
		double val = network->activate(explore_obs_histories[h_index]);
		explore_remaining_vals[h_index] = explore_target_val_histories[h_index] - val;
	}

	double existing_existing_sum_misguess = 0.0;
	for (int h_index = num_train_samples; h_index < (int)existing_obs_histories.size(); h_index++) {
		existing_existing_sum_misguess += existing_remaining_vals[h_index] * existing_remaining_vals[h_index];
	}

	double explore_existing_sum_misguess = 0.0;
	for (int h_index = num_train_samples; h_index < (int)explore_obs_histories.size(); h_index++) {
		explore_existing_sum_misguess += explore_remaining_vals[h_index] * explore_remaining_vals[h_index];
	}

	double existing_sum_misguess = (existing_existing_sum_misguess + explore_existing_sum_misguess) / 2.0;

	BuildNetwork* best_network = NULL;
	double best_sum_misguess = numeric_limits<double>::max();

	uniform_int_distribution<int> existing_distribution(0, 1);
	for (int try_index = 0; try_index < 10; try_index++) {
		vector<int> possible_inputs;
		for (int i_index = 0; i_index < (int)explore_obs_histories[0].size(); i_index++) {
			possible_inputs.push_back(i_index);
		}

		vector<int> input_indexes;
		while (true) {
			if (possible_inputs.size() == 0) {
				break;
			}

			uniform_int_distribution<int> distribution(0, possible_inputs.size()-1);
			int index = distribution(generator);

			input_indexes.push_back(possible_inputs[index]);

			possible_inputs.erase(possible_inputs.begin() + index);

			if (input_indexes.size() >= BUILD_MAX_NUM_INPUTS) {
				break;
			}
		}
		BuildNode* curr_node = new BuildNode(input_indexes);

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
			if (existing_distribution(generator)) {
				int index = train_distribution(generator);

				curr_node->activate(existing_obs_histories[index]);

				double error = existing_remaining_vals[index] - curr_node->output->acti_vals[0];
				curr_node->output->errors[0] = error;
				curr_node->backprop();
			} else {
				int index = train_distribution(generator);

				curr_node->activate(explore_obs_histories[index]);

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

		BuildNetwork* curr_network = new BuildNetwork();
		curr_network->copy_from(network);

		if (curr_network->nodes.size() == 0) {
			curr_network->inputs = vector<double>(explore_obs_histories[0].size());
		}

		curr_network->nodes.push_back(curr_node);

		curr_network->output_weights.push_back(1.0);
		curr_network->output_weight_updates.push_back(0.0);

		for (int iter_index = 0; iter_index < REFINE_ITERS; iter_index++) {
			if (existing_distribution(generator)) {
				int index = train_distribution(generator);
				curr_network->backprop(existing_obs_histories[index],
									   existing_target_val_histories[index]);
			} else {
				int index = train_distribution(generator);
				curr_network->backprop(explore_obs_histories[index],
									   explore_target_val_histories[index]);
			}
		}

		double existing_curr_sum_misguess = 0.0;
		for (int h_index = num_train_samples; h_index < (int)existing_obs_histories.size(); h_index++) {
			double val = curr_network->activate(existing_obs_histories[h_index]);

			existing_curr_sum_misguess += (existing_target_val_histories[h_index] - val)
				* (existing_target_val_histories[h_index] - val);
		}

		double explore_curr_sum_misguess = 0.0;
		for (int h_index = num_train_samples; h_index < (int)explore_obs_histories.size(); h_index++) {
			double val = curr_network->activate(explore_obs_histories[h_index]);

			explore_curr_sum_misguess += (explore_target_val_histories[h_index] - val)
				* (explore_target_val_histories[h_index] - val);
		}

		double curr_sum_misguess = (existing_curr_sum_misguess + explore_curr_sum_misguess) / 2.0;

		if (curr_sum_misguess < existing_sum_misguess && curr_sum_misguess < best_sum_misguess) {
			if (best_network != NULL) {
				delete best_network;
			}
			best_network = curr_network;
			best_sum_misguess = curr_sum_misguess;
		} else {
			delete curr_network;
		}
	}

	if (best_sum_misguess < existing_sum_misguess) {
		existing_sum_misguess = best_sum_misguess;

		delete network;
		network = best_network;
	}

	for (int n_index = (int)network->nodes.size()-1; n_index >= 0; n_index--) {
		clean_network_helper(existing_obs_histories,
							 existing_target_val_histories,
							 explore_obs_histories,
							 explore_target_val_histories,
							 existing_sum_misguess,
							 network,
							 n_index);
	}
}
