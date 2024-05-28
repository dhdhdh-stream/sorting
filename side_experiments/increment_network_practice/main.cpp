#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "constants.h"
#include "globals.h"
#include "network.h"
#include "nn_helpers.h"

using namespace std;

int seed;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	vector<int> network_input_indexes;
	Network* network = NULL;
	double average_misguess;
	double misguess_standard_deviation;

	uniform_int_distribution<int> x_distribution(0, 4);
	uniform_int_distribution<int> y_distribution(0, 4);
	for (int epoch_iter = 0; epoch_iter < 100; epoch_iter++) {
		vector<int> test_indexes = network_input_indexes;

		vector<int> remaining_indexes(100);
		for (int p_index = 0; p_index < 100; p_index++) {
			remaining_indexes[p_index] = p_index;
		}
		int num_new_input = 0;
		while (true) {
			uniform_int_distribution<int> distribution(0, (int)remaining_indexes.size()-1);
			int rand_index = distribution(generator);

			bool contains = false;
			for (int t_index = 0; t_index < (int)test_indexes.size(); t_index++) {
				if (test_indexes[t_index] == remaining_indexes[rand_index]) {
					contains = true;
					break;
				}
			}
			if (!contains) {
				test_indexes.push_back(remaining_indexes[rand_index]);
				num_new_input++;
			}

			remaining_indexes.erase(remaining_indexes.begin() + rand_index);

			if (num_new_input >= NETWORK_INCREMENT_NUM_NEW
					|| remaining_indexes.size() == 0) {
				break;
			}
		}

		Network* test_network;
		if (network == NULL) {
			test_network = new Network();
		} else {
			test_network = new Network(network);
		}
		test_network->increment((int)test_indexes.size());

		vector<vector<double>> inputs;
		vector<double> target_vals;
		for (int iter_index = 0; iter_index < NUM_DATAPOINTS; iter_index++) {
			int start_x = x_distribution(generator);
			int start_y = y_distribution(generator);

			int end_x = x_distribution(generator);
			int end_y = y_distribution(generator);

			double x_distance = end_x - start_x;
			double y_distance = end_y - end_x;
			double distance = sqrt(x_distance * x_distance + y_distance * y_distance);

			target_vals.push_back(distance);

			vector<double> input_vals(100);
			for (int x_index = 0; x_index < 5; x_index++) {
				for (int y_index = 0; y_index < 5; y_index++) {
					if (x_index == start_x
							&& y_index == start_y) {
						input_vals[5*x_index + y_index] = 1.0;
					} else {
						input_vals[5*x_index + y_index] = 0.0;
					}
				}
			}
			for (int x_index = 0; x_index < 5; x_index++) {
				for (int y_index = 0; y_index < 5; y_index++) {
					if (x_index == end_x
							&& y_index == end_y) {
						input_vals[25 + 5*x_index + y_index] = 1.0;
					} else {
						input_vals[25 + 5*x_index + y_index] = 0.0;
					}
				}
			}
			for (int i_index = 50; i_index < 100; i_index++) {
				input_vals[i_index] = rand()%2;
			}

			vector<double> curr_input_vals(test_indexes.size());
			for (int i_index = 0; i_index < (int)test_indexes.size(); i_index++) {
				curr_input_vals[i_index] = input_vals[test_indexes[i_index]];
			}

			inputs.push_back(curr_input_vals);
		}

		train_network(inputs,
					  target_vals,
					  test_indexes,
					  test_network);

		double test_average_misguess;
		double test_misguess_standard_deviation;
		measure_network(inputs,
						target_vals,
						test_network,
						test_average_misguess,
						test_misguess_standard_deviation);

		cout << "test_average_misguess: " << test_average_misguess << endl;
		cout << "test_misguess_standard_deviation: " << test_misguess_standard_deviation << endl;

		bool is_select = false;
		if (network == NULL) {
			is_select = true;
		} else {
			double improvement = average_misguess - test_average_misguess;
			double standard_deviation = min(misguess_standard_deviation, test_misguess_standard_deviation);
			double t_score = improvement / (standard_deviation / sqrt(NUM_DATAPOINTS * TEST_SAMPLES_PERCENTAGE));

			if (t_score > 1.645) {
				is_select = true;
			}
		}

		if (is_select) {
			cout << "update" << endl;

			average_misguess = test_average_misguess;
			misguess_standard_deviation = test_misguess_standard_deviation;

			int original_input_indexes_size = (int)network_input_indexes.size();
			network_input_indexes = test_indexes;

			if (network != NULL) {
				delete network;
			}
			network = test_network;

			/**
			 * - only clean new as old might have been dependent on previous dependents
			 */
			int starting_input_indexes_size = (int)network_input_indexes.size();
			for (int i_index = starting_input_indexes_size-1; i_index >= original_input_indexes_size; i_index--) {
				vector<int> remove_test_indexes = network_input_indexes;
				Network* remove_test_network = new Network(network);

				remove_test_indexes.erase(remove_test_indexes.begin() + i_index);

				remove_test_network->input->acti_vals.erase(remove_test_network->input->acti_vals.begin() + i_index);
				remove_test_network->input->errors.erase(remove_test_network->input->errors.begin() + i_index);

				for (int l_index = 0; l_index < (int)remove_test_network->hiddens.size(); l_index++) {
					remove_test_network->hiddens[l_index]->remove_input(i_index);
				}
				remove_test_network->output->remove_input(i_index);

				vector<vector<double>> remove_test_inputs = inputs;
				for (int d_index = 0; d_index < (int)remove_test_inputs.size(); d_index++) {
					remove_test_inputs[d_index].erase(remove_test_inputs[d_index].begin() + i_index);
				}

				optimize_network(remove_test_inputs,
								 target_vals,
								 remove_test_network);

				double remove_test_average_misguess;
				double remove_test_misguess_standard_deviation;
				measure_network(remove_test_inputs,
								target_vals,
								remove_test_network,
								remove_test_average_misguess,
								remove_test_misguess_standard_deviation);

				double remove_improvement = average_misguess - remove_test_average_misguess;
				double remove_standard_deviation = min(misguess_standard_deviation, remove_test_misguess_standard_deviation);
				double remove_t_score = remove_improvement / (remove_standard_deviation / sqrt(NUM_DATAPOINTS * TEST_SAMPLES_PERCENTAGE));

				cout << "test_indexes[i_index]: " << test_indexes[i_index] << endl;

				if (remove_t_score > -0.674) {
					cout << "cleaned" << endl;
					network_input_indexes = remove_test_indexes;
					delete network;
					network = remove_test_network;
					inputs = remove_test_inputs;
				} else {
					delete remove_test_network;
				}
			}

			measure_network(inputs,
							target_vals,
							network,
							average_misguess,
							misguess_standard_deviation);

			cout << "average_misguess: " << average_misguess << endl;
			cout << "misguess_standard_deviation: " << misguess_standard_deviation << endl;
		} else {
			delete test_network;
		}

		cout << "network_input_indexes:" << endl;
		for (int i_index = 0; i_index < (int)network_input_indexes.size(); i_index++) {
			cout << network_input_indexes[i_index] << endl;
		}
		cout << "network->input->acti_vals.size(): " << network->input->acti_vals.size() << endl;
		cout << "network->hiddens.size(): " << network->hiddens.size() << endl;
		for (int l_index = 0; l_index < (int)network->hiddens.size(); l_index++) {
			cout << l_index << ": " << network->hiddens[l_index]->acti_vals.size() << endl;
		}
		cout << endl;

		if (average_misguess < 0.01) {
			break;
		}
	}

	for (int iter_index = 0; iter_index < 20; iter_index++) {
		int start_x = x_distribution(generator);
		int start_y = y_distribution(generator);

		int end_x = x_distribution(generator);
		int end_y = y_distribution(generator);

		double x_distance = end_x - start_x;
		double y_distance = end_y - end_x;
		double distance = sqrt(x_distance * x_distance + y_distance * y_distance);

		vector<double> input_vals(100);
		for (int x_index = 0; x_index < 5; x_index++) {
			for (int y_index = 0; y_index < 5; y_index++) {
				if (x_index == start_x
						&& y_index == start_y) {
					input_vals[5*x_index + y_index] = 1.0;
				} else {
					input_vals[5*x_index + y_index] = 0.0;
				}
			}
		}
		for (int x_index = 0; x_index < 5; x_index++) {
			for (int y_index = 0; y_index < 5; y_index++) {
				if (x_index == end_x
						&& y_index == end_y) {
					input_vals[25 + 5*x_index + y_index] = 1.0;
				} else {
					input_vals[25 + 5*x_index + y_index] = 0.0;
				}
			}
		}
		for (int i_index = 50; i_index < 100; i_index++) {
			input_vals[i_index] = rand()%2;
		}

		vector<double> curr_input_vals(network_input_indexes.size());
		for (int i_index = 0; i_index < (int)network_input_indexes.size(); i_index++) {
			curr_input_vals[i_index] = input_vals[network_input_indexes[i_index]];
		}

		network->activate(curr_input_vals);

		cout << "iter_index: " << iter_index << endl;
		cout << "start_x: " << start_x << endl;
		cout << "start_y: " << start_y << endl;
		cout << "end_x: " << end_x << endl;
		cout << "end_y: " << end_y << endl;
		cout << "distance: " << distance << endl;
		cout << "network->output->acti_vals[0]: " << network->output->acti_vals[0] << endl;
		cout << endl;
	}

	cout << "network_input_indexes:" << endl;
	for (int i_index = 0; i_index < (int)network_input_indexes.size(); i_index++) {
		cout << network_input_indexes[i_index] << endl;
	}
	cout << "network->input->acti_vals.size(): " << network->input->acti_vals.size() << endl;
	cout << "network->hiddens.size(): " << network->hiddens.size() << endl;
	for (int l_index = 0; l_index < (int)network->hiddens.size(); l_index++) {
		cout << l_index << ": " << network->hiddens[l_index]->acti_vals.size() << endl;
	}
	cout << endl;

	// test increment
	{
		network->increment(10);

		for (int iter_index = 0; iter_index < 20; iter_index++) {
			int start_x = x_distribution(generator);
			int start_y = y_distribution(generator);

			int end_x = x_distribution(generator);
			int end_y = y_distribution(generator);

			double x_distance = end_x - start_x;
			double y_distance = end_y - end_x;
			double distance = sqrt(x_distance * x_distance + y_distance * y_distance);

			vector<double> input_vals(100);
			for (int x_index = 0; x_index < 5; x_index++) {
				for (int y_index = 0; y_index < 5; y_index++) {
					if (x_index == start_x
							&& y_index == start_y) {
						input_vals[5*x_index + y_index] = 1.0;
					} else {
						input_vals[5*x_index + y_index] = 0.0;
					}
				}
			}
			for (int x_index = 0; x_index < 5; x_index++) {
				for (int y_index = 0; y_index < 5; y_index++) {
					if (x_index == end_x
							&& y_index == end_y) {
						input_vals[25 + 5*x_index + y_index] = 1.0;
					} else {
						input_vals[25 + 5*x_index + y_index] = 0.0;
					}
				}
			}
			for (int i_index = 50; i_index < 100; i_index++) {
				input_vals[i_index] = rand()%2;
			}

			vector<double> curr_input_vals(network_input_indexes.size());
			for (int i_index = 0; i_index < (int)network_input_indexes.size(); i_index++) {
				curr_input_vals[i_index] = input_vals[network_input_indexes[i_index]];
			}
			for (int i = 0; i < 10; i++) {
				curr_input_vals.push_back(rand()%2);
			}

			network->activate(curr_input_vals);

			cout << "iter_index: " << iter_index << endl;
			cout << "start_x: " << start_x << endl;
			cout << "start_y: " << start_y << endl;
			cout << "end_x: " << end_x << endl;
			cout << "end_y: " << end_y << endl;
			cout << "distance: " << distance << endl;
			cout << "network->output->acti_vals[0]: " << network->output->acti_vals[0] << endl;
			cout << endl;
		}
	}

	cout << "Done" << endl;
}
