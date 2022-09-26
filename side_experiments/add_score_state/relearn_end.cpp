#include <chrono>
#include <iostream>
#include <thread>
#include <random>

#include "fold_network.h"
#include "network.h"
#include "node.h"
#include "test_node.h"

using namespace std;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	vector<Node*> nodes;
	for (int i = 0; i < 10; i++) {
		ifstream input_file;
		input_file.open("saves/p2_n_" + to_string(i) + "_9.txt");
		nodes.push_back(new Node("p2_n_"+to_string(i), input_file));
		input_file.close();
	}

	Network* network = new Network(2, 8, 1);

	double sum_error = 0.0;
	for (int epoch_index = 1; epoch_index < 5000; epoch_index++) {
		if (epoch_index%100 == 0) {
			cout << endl;
			cout << epoch_index << endl;
			cout << "sum_error: " << sum_error << endl;
			sum_error = 0.0;
		}

		for (int iter_index = 0; iter_index < 100; iter_index++) {
			vector<vector<double>> state_vals;
			vector<bool> scopes_on;

			int flat_input_counter = 0;

			// first block
			int first_block_on_index = rand()%5;
			nodes[flat_input_counter]->activate(state_vals,
												scopes_on,
												first_block_on_index);
			flat_input_counter++;

			int first_block_sum = 0;
			for (int i = 0; i < 4; i++) {
				int value = rand()%4;
				nodes[flat_input_counter]->activate(state_vals,
													scopes_on,
													value);
				flat_input_counter++;
				if (i < first_block_on_index) {
					first_block_sum += value;
				}
			}

			// second block
			int second_block_on_index = rand()%5;
			nodes[flat_input_counter]->activate(state_vals,
												scopes_on,
												second_block_on_index);
			flat_input_counter++;

			int second_block_sum = 0;
			for (int i = 0; i < 4; i++) {
				int value = rand()%4;
				nodes[flat_input_counter]->activate(state_vals,
													scopes_on,
													value);
				flat_input_counter++;
				if (i < second_block_on_index) {
					second_block_sum += value;
				}
			}

			int final_value = first_block_sum - second_block_sum;

			vector<double> inputs;
			inputs.push_back(rand()%4);
			inputs.push_back(state_vals[0][0]);

			network->activate(inputs);

			vector<double> errors;
			errors.push_back(final_value - network->output->acti_vals[0]);
			sum_error += abs(errors[0]);

			network->backprop(errors);
		}

		if (epoch_index < 2000) {
			double max_update = 0.0;
			network->calc_max_update(max_update, 0.001);
			double factor = 1.0;
			if (max_update > 0.01) {
				factor = 0.01/max_update;
			}
			network->update_weights(factor, 0.001);
		} else {
			double max_update = 0.0;
			network->calc_max_update(max_update, 0.0001);
			double factor = 1.0;
			if (max_update > 0.001) {
				factor = 0.001/max_update;
			}
			network->update_weights(factor, 0.0001);
		}
	}

	for (int n_index = 0; n_index < (int)nodes.size(); n_index++) {
		delete nodes[n_index];
	}
	delete network;

	cout << "Done" << endl;
}
