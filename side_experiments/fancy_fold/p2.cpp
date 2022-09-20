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

bool zero_train_step(int input_counter,
					 int zero_fold_index,
					 int fold_index,
					 double observation,
					 vector<vector<double>>& state_vals,
					 vector<vector<double>>& zero_train_state_vals,
					 vector<Node*>& nodes,
					 TestNode& test_node) {
	if (input_counter < zero_fold_index) {
		nodes[input_counter]->activate(state_vals, observation);
		return false;
	} else if (input_counter == zero_fold_index) {
		nodes[input_counter]->activate(state_vals, observation);
		zero_train_state_vals = state_vals;
		for (int sc_index = 0; sc_index < (int)zero_train_state_vals.size()-1; sc_index++) {
			for (int st_index = 0; st_index < (int)zero_train_state_vals[sc_index].size(); st_index++) {
				zero_train_state_vals[sc_index][st_index] = 0.0;
			}
		}
		return false;
	} else if (input_counter < fold_index) {
		nodes[input_counter]->activate_zero_train(
			state_vals,
			observation,
			zero_train_state_vals);
		return false;
	} else { 
		// fold_index == input_counter
		test_node.process_zero_train(state_vals,
									 zero_train_state_vals,
									 observation);
		return true;
	}
}

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	// FoldNetwork* fold_network = new FoldNetwork(10, 1);

	// double sum_error = 0.0;
	// for (int epoch_index = 1; epoch_index < 50000; epoch_index++) {
	// 	if (epoch_index%100 == 0) {
	// 		cout << endl;
	// 		cout << epoch_index << endl;
	// 		cout << "sum_error: " << sum_error << endl;
	// 		sum_error = 0.0;
	// 	}

	// 	for (int iter_index = 0; iter_index < 100; iter_index++) {

	// 		double flat_inputs[10];
	// 		int flat_input_counter = 0;
	// 		bool activated[10];
	// 		for (int i = 0; i < 10; i++) {
	// 			activated[i] = true;
	// 		}

	// 		// first block
	// 		int first_block_on_index = rand()%5;
	// 		flat_inputs[flat_input_counter] = first_block_on_index;
	// 		flat_input_counter++;

	// 		int first_block_sum = 0;
	// 		for (int i = 0; i < 4; i++) {
	// 			int value = rand()%4;
	// 			flat_inputs[flat_input_counter] = value;
	// 			flat_input_counter++;
	// 			if (i < first_block_on_index) {
	// 				first_block_sum += value;
	// 			}
	// 		}

	// 		// second block
	// 		int second_block_on_index = rand()%5;
	// 		flat_inputs[flat_input_counter] = second_block_on_index;
	// 		flat_input_counter++;

	// 		int second_block_sum = 0;
	// 		for (int i = 0; i < 4; i++) {
	// 			int value = rand()%4;
	// 			flat_inputs[flat_input_counter] = value;
	// 			flat_input_counter++;
	// 			if (i < second_block_on_index) {
	// 				second_block_sum += value;
	// 			}
	// 		}

	// 		int final_value = first_block_sum - second_block_sum;

	// 		vector<double> obs;
	// 		obs.push_back(rand()%4);

	// 		fold_network->activate(flat_inputs,
	// 							   activated,
	// 							   obs);

	// 		vector<double> errors;
	// 		errors.push_back(final_value - fold_network->output->acti_vals[0]);
	// 		sum_error += abs(errors[0]);

	// 		fold_network->backprop(errors);
	// 	}
	// }

	// ofstream output_file;
	// output_file.open("saves/fold_network.txt");
	// fold_network->save(output_file);
	// output_file.close();

	// ifstream input_file;
	// input_file.open("saves/fold_network.txt");
	// FoldNetwork* fold_network = new FoldNetwork(input_file);
	// input_file.close();

	// double average_error = 0.0;
	// for (int iter_index = 0; iter_index < 10000; iter_index++) {
	// 	double flat_inputs[10];
	// 	int flat_input_counter = 0;
	// 	bool activated[10];
	// 	for (int i = 0; i < 10; i++) {
	// 		activated[i] = true;
	// 	}

	// 	// first block
	// 	int first_block_on_index = rand()%5;
	// 	flat_inputs[flat_input_counter] = first_block_on_index;
	// 	flat_input_counter++;

	// 	int first_block_sum = 0;
	// 	for (int i = 0; i < 4; i++) {
	// 		int value = rand()%4;
	// 		flat_inputs[flat_input_counter] = value;
	// 		flat_input_counter++;
	// 		if (i < first_block_on_index) {
	// 			first_block_sum += value;
	// 		}
	// 	}

	// 	// second block
	// 	int second_block_on_index = rand()%5;
	// 	flat_inputs[flat_input_counter] = second_block_on_index;
	// 	flat_input_counter++;

	// 	int second_block_sum = 0;
	// 	for (int i = 0; i < 4; i++) {
	// 		int value = rand()%4;
	// 		flat_inputs[flat_input_counter] = value;
	// 		flat_input_counter++;
	// 		if (i < second_block_on_index) {
	// 			second_block_sum += value;
	// 		}
	// 	}

	// 	int final_value = first_block_sum - second_block_sum;

	// 	vector<double> obs;
	// 	obs.push_back(rand()%2);

	// 	fold_network->activate(flat_inputs,
	// 						   activated,
	// 						   obs);

	// 	average_error += abs(final_value - fold_network->output->acti_vals[0]);
	// }

	// average_error /= 10000;
	// cout << "average_error: " << average_error << endl;

	ifstream input_file;
	input_file.open("saves/fold_network.txt");
	// input_file.open("saves/f_1.txt");
	FoldNetwork* fold_network = new FoldNetwork(input_file);
	input_file.close();

	double target_error = 0.3;

	vector<Node*> nodes;
	// for (int i = 0; i < 2; i++) {
	// 	ifstream input_file;
	// 	input_file.open("saves/n_" + to_string(i) + ".txt");
	// 	nodes.push_back(new Node(i, input_file));
	// 	input_file.close();
	// }
	vector<int> scope_sizes;
	for (int n_index = 0; n_index < (int)nodes.size(); n_index++) {
		nodes[n_index]->get_scope_sizes(scope_sizes);
	}
	int fold_index = (int)nodes.size();

	for (int compress_index = fold_index; compress_index < 10; compress_index++) {
		TestNode test_node(target_error,
						   scope_sizes,
						   fold_network);
		while (true) {
			int rand_non_zero_index = 0;	// # of scopes to include as non_zero
			if (test_node.state == STATE_LOCAL_SCOPE_ZERO_TRAIN
					|| test_node.state == STATE_COMPRESS_ZERO_TRAIN) {
				// zero only from before compression
				rand_non_zero_index = rand()%(int)test_node.test_scope_sizes.size();
			}

			if (rand_non_zero_index != 0) {
				int zero_fold_index;
				int curr_layer = 0;
				for (int n_index = (int)nodes.size()-1; n_index >= 0; n_index--) {
					curr_layer -= (int)nodes[n_index]->compression_networks.size();

					if (!nodes[n_index]->update_existing_scope) {
						curr_layer++;
					}

					// bookkeep from before compression
					if (curr_layer == rand_non_zero_index+(int)test_node.compression_networks.size()) {
						zero_fold_index = n_index;
					}
				}

				vector<vector<double>> state_vals;
				vector<vector<double>> zero_train_state_vals;
				bool halt = false;

				double flat_inputs[10];
				int flat_input_counter = 0;

				// first block
				int first_block_on_index = rand()%5;
				flat_inputs[flat_input_counter] = first_block_on_index;
				halt = zero_train_step(flat_input_counter,
									   zero_fold_index,
									   fold_index,
									   flat_inputs[flat_input_counter],
									   state_vals,
									   zero_train_state_vals,
									   nodes,
									   test_node);
				if (halt) {
					continue;
				}
				flat_input_counter++;

				int first_block_sum = 0;
				for (int i = 0; i < 4; i++) {
					int value = rand()%4;
					flat_inputs[flat_input_counter] = value;
					if (i < first_block_on_index) {
						first_block_sum += value;
					}
					halt = zero_train_step(flat_input_counter,
										   zero_fold_index,
										   fold_index,
										   flat_inputs[flat_input_counter],
										   state_vals,
										   zero_train_state_vals,
										   nodes,
										   test_node);
					if (halt) {
						break;
					}
					flat_input_counter++;
				}
				if (halt) {
					continue;
				}

				// second block
				// first block
				int second_block_on_index = rand()%5;
				flat_inputs[flat_input_counter] = second_block_on_index;
				halt = zero_train_step(flat_input_counter,
									   zero_fold_index,
									   fold_index,
									   flat_inputs[flat_input_counter],
									   state_vals,
									   zero_train_state_vals,
									   nodes,
									   test_node);
				if (halt) {
					continue;
				}
				flat_input_counter++;

				int second_block_sum = 0;
				for (int i = 0; i < 4; i++) {
					int value = rand()%4;
					flat_inputs[flat_input_counter] = value;
					if (i < second_block_on_index) {
						second_block_sum += value;
					}
					halt = zero_train_step(flat_input_counter,
										   zero_fold_index,
										   fold_index,
										   flat_inputs[flat_input_counter],
										   state_vals,
										   zero_train_state_vals,
										   nodes,
										   test_node);
					if (halt) {
						break;
					}
					flat_input_counter++;
				}
				if (halt) {
					continue;
				}

				// don't activate fold on zero train
			} else {
				vector<vector<double>> state_vals;
				vector<vector<double>> test_state_vals;

				double flat_inputs[10];
				int flat_input_counter = 0;
				bool activated[10];
				for (int i = 0; i < 10; i++) {
					activated[i] = true;
				}

				// first block
				int first_block_on_index = rand()%5;
				flat_inputs[flat_input_counter] = first_block_on_index;
				if (fold_index > flat_input_counter) {
					nodes[flat_input_counter]->activate(state_vals,
														flat_inputs[flat_input_counter]);
				} else if (fold_index == flat_input_counter) {
					test_node.activate(state_vals,
									   flat_inputs[flat_input_counter],
									   test_state_vals);
				}
				flat_input_counter++;

				int first_block_sum = 0;
				for (int i = 0; i < 4; i++) {
					int value = rand()%4;
					flat_inputs[flat_input_counter] = value;
					if (i < first_block_on_index) {
						first_block_sum += value;
					}
					if (fold_index > flat_input_counter) {
						nodes[flat_input_counter]->activate(state_vals,
															flat_inputs[flat_input_counter]);
					} else if (fold_index == flat_input_counter) {
						test_node.activate(state_vals,
										   flat_inputs[flat_input_counter],
										   test_state_vals);
					}
					flat_input_counter++;
				}

				// second block
				int second_block_on_index = rand()%5;
				flat_inputs[flat_input_counter] = second_block_on_index;
				if (fold_index > flat_input_counter) {
					nodes[flat_input_counter]->activate(state_vals,
														flat_inputs[flat_input_counter]);
				} else if (fold_index == flat_input_counter) {
					test_node.activate(state_vals,
									   flat_inputs[flat_input_counter],
									   test_state_vals);
				}
				flat_input_counter++;

				int second_block_sum = 0;
				for (int i = 0; i < 4; i++) {
					int value = rand()%4;
					flat_inputs[flat_input_counter] = value;
					if (i < second_block_on_index) {
						second_block_sum += value;
					}
					if (fold_index > flat_input_counter) {
						nodes[flat_input_counter]->activate(state_vals,
															flat_inputs[flat_input_counter]);
					} else if (fold_index == flat_input_counter) {
						test_node.activate(state_vals,
										   flat_inputs[flat_input_counter],
										   test_state_vals);
					}
					flat_input_counter++;
				}

				int final_value = first_block_sum - second_block_sum;

				test_node.process(flat_inputs,
								  activated,
								  state_vals,
								  test_state_vals,
								  rand()%4,
								  final_value);
			}

			if (test_node.state == STATE_DONE) {
				break;
			}
		}

		nodes.push_back(new Node(fold_index,
								 test_node.outputs_state,
								 test_node.update_existing_scope,
								 1,
								 test_node.state_network,
								 test_node.compression_networks));

		{
			ofstream output_file;
			output_file.open("saves/n_" + to_string(nodes.back()->id) + ".txt");
			nodes.back()->save(output_file);
			output_file.close();
		}

		delete fold_network;
		fold_network = test_node.curr_fold;

		{
			ofstream output_file;
			output_file.open("saves/f_" + to_string(nodes.back()->id) + ".txt");
			fold_network->save(output_file);
			output_file.close();
		}

		cout << "SAVED " << nodes.back()->id << endl;

		scope_sizes = test_node.curr_scope_sizes;

		fold_index++;
	}

	for (int n_index = 0; n_index < (int)nodes.size(); n_index++) {
		delete nodes[n_index];
	}
	delete fold_network;

	cout << "Done" << endl;
}
