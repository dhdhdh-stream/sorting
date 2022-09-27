// zero train currently prevents convergence(?)
// only zero train after fully converged(?)

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

void zero_train_step(int input_counter,
					 int zero_fold_index,
					 int fold_index,
					 double observation,
					 vector<vector<double>>& state_vals,
					 vector<bool>& scopes_on,
					 vector<Node*>& nodes,
					 TestNode* test_node) {
	if (input_counter < zero_fold_index) {
		nodes[input_counter]->activate(state_vals,
									   scopes_on,
									   observation);
	} else if (input_counter == zero_fold_index) {
		nodes[input_counter]->activate(state_vals,
									   scopes_on,
									   observation);
		for (int sc_index = 0; sc_index < (int)state_vals.size()-1; sc_index++) {
			for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
				state_vals[sc_index][st_index] = 0.0;
			}
			scopes_on[sc_index] = false;
		}
	} else if (input_counter < fold_index) {
		nodes[input_counter]->activate(state_vals,
									   scopes_on,
									   observation);
	} else if (fold_index == input_counter) { 
		test_node->activate(state_vals,
							scopes_on,
							observation);
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
	// for (int epoch_index = 1; epoch_index < 10000; epoch_index++) {
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

	// 		fold_network->activate_leaky(flat_inputs,
	// 									 activated,
	// 									 obs);

	// 		vector<double> errors;
	// 		errors.push_back(final_value - fold_network->output->acti_vals[0]);
	// 		sum_error += abs(errors[0]);

	// 		fold_network->backprop_leaky(errors);
	// 	}

	// 	if (epoch_index < 5000) {
	// 		double max_update = 0.0;
	// 		fold_network->calc_max_update_leaky(max_update, 0.001);
	// 		double factor = 1.0;
	// 		if (max_update > 0.01) {
	// 			factor = 0.01/max_update;
	// 		}
	// 		fold_network->update_weights_leaky(factor, 0.001);
	// 	} else {
	// 		double max_update = 0.0;
	// 		fold_network->calc_max_update_leaky(max_update, 0.0001);
	// 		double factor = 1.0;
	// 		if (max_update > 0.001) {
	// 			factor = 0.001/max_update;
	// 		}
	// 		fold_network->update_weights_leaky(factor, 0.0001);
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
	input_file.open("saves/p2_fold_network.txt");
	FoldNetwork* fold_network = new FoldNetwork(input_file);
	input_file.close();

	double target_error = 0.25;

	vector<Node*> nodes;
	// for (int i = 0; i < 4; i++) {
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
		TestNode* test_node = new TestNode(target_error,
										   scope_sizes,
										   fold_network);
		while (true) {
			int rand_non_zero_index = 0;	// # of scopes to include as non_zero
			if (test_node->state == STATE_LOCAL_SCOPE_LEARN
					|| test_node->state == STATE_LOCAL_SCOPE_TUNE
					|| test_node->state == STATE_COMPRESS_LEARN
					|| test_node->state == STATE_COMPRESS_TUNE) {
				if (rand()%2 == 0) {
					// zero only from before compression
					rand_non_zero_index = rand()%(int)test_node->test_scope_sizes.size();
				}
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
					if (curr_layer == rand_non_zero_index+(int)test_node->compression_networks.size()) {
						zero_fold_index = n_index;
					}
				}

				vector<vector<double>> state_vals;
				vector<bool> scopes_on;

				double flat_inputs[10];
				int flat_input_counter = 0;
				bool activated[10];
				for (int i = 0; i < 10; i++) {
					activated[i] = true;
				}

				// first block
				int first_block_on_index = rand()%5;
				flat_inputs[flat_input_counter] = first_block_on_index;
				zero_train_step(flat_input_counter,
								zero_fold_index,
								fold_index,
								flat_inputs[flat_input_counter],
								state_vals,
								scopes_on,
								nodes,
								test_node);
				flat_input_counter++;

				int first_block_sum = 0;
				for (int i = 0; i < 4; i++) {
					int value = rand()%4;
					flat_inputs[flat_input_counter] = value;
					if (i < first_block_on_index) {
						first_block_sum += value;
					}
					zero_train_step(flat_input_counter,
									zero_fold_index,
									fold_index,
									flat_inputs[flat_input_counter],
									state_vals,
									scopes_on,
									nodes,
									test_node);
					flat_input_counter++;
				}

				// second block
				int second_block_on_index = rand()%5;
				flat_inputs[flat_input_counter] = second_block_on_index;
				zero_train_step(flat_input_counter,
								zero_fold_index,
								fold_index,
								flat_inputs[flat_input_counter],
								state_vals,
								scopes_on,
								nodes,
								test_node);
				flat_input_counter++;

				int second_block_sum = 0;
				for (int i = 0; i < 4; i++) {
					int value = rand()%4;
					flat_inputs[flat_input_counter] = value;
					if (i < second_block_on_index) {
						second_block_sum += value;
					}
					zero_train_step(flat_input_counter,
									zero_fold_index,
									fold_index,
									flat_inputs[flat_input_counter],
									state_vals,
									scopes_on,
									nodes,
									test_node);
					flat_input_counter++;
				}

				int final_value = first_block_sum - second_block_sum;

				test_node->process(flat_inputs,
								   activated,
								   state_vals,
								   rand()%4,
								   final_value,
								   true,
								   nodes);
			} else {
				vector<vector<double>> state_vals;
				vector<bool> scopes_on;

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
														scopes_on,
														flat_inputs[flat_input_counter]);
				} else if (fold_index == flat_input_counter) {
					test_node->activate(state_vals,
										scopes_on,
										flat_inputs[flat_input_counter]);
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
															scopes_on,
															flat_inputs[flat_input_counter]);
					} else if (fold_index == flat_input_counter) {
						test_node->activate(state_vals,
											scopes_on,
											flat_inputs[flat_input_counter]);
					}
					flat_input_counter++;
				}

				// second block
				int second_block_on_index = rand()%5;
				flat_inputs[flat_input_counter] = second_block_on_index;
				if (fold_index > flat_input_counter) {
					nodes[flat_input_counter]->activate(state_vals,
														scopes_on,
														flat_inputs[flat_input_counter]);
				} else if (fold_index == flat_input_counter) {
					test_node->activate(state_vals,
										scopes_on,
										flat_inputs[flat_input_counter]);
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
															scopes_on,
															flat_inputs[flat_input_counter]);
					} else if (fold_index == flat_input_counter) {
						test_node->activate(state_vals,
											scopes_on,
											flat_inputs[flat_input_counter]);
					}
					flat_input_counter++;
				}

				int final_value = first_block_sum - second_block_sum;

				test_node->process(flat_inputs,
								   activated,
								   state_vals,
								   rand()%4,
								   final_value,
								   false,
								   nodes);
			}

			if (test_node->state == STATE_DONE) {
				break;
			}
		}

		nodes.push_back(new Node("p2_n_"+to_string(fold_index),
								 test_node->outputs_state,
								 test_node->update_existing_scope,
								 1,
								 test_node->state_network,
								 test_node->compression_networks,
								 test_node->compressed_scope_sizes));

		for (int n_index = 0; n_index < (int)nodes.size(); n_index++) {
			ofstream output_file;
			output_file.open("saves/" + nodes[n_index]->id + "_" + to_string(compress_index) + ".txt");
			nodes[n_index]->save(output_file);
			output_file.close();
		}

		delete fold_network;
		fold_network = test_node->curr_fold;

		{
			ofstream output_file;
			output_file.open("saves/p2_f_" + to_string(compress_index) + ".txt");
			fold_network->save(output_file);
			output_file.close();
		}

		cout << "SAVED " << nodes.back()->id << endl;

		scope_sizes = test_node->curr_scope_sizes;

		fold_index++;
	}

	for (int n_index = 0; n_index < (int)nodes.size(); n_index++) {
		delete nodes[n_index];
	}
	delete fold_network;

	cout << "Done" << endl;
}