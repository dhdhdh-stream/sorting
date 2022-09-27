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

	// FoldNetwork* fold_network = new FoldNetwork(10, 1);

	// double sum_error = 0.0;
	// for (int epoch_index = 1; epoch_index < 20000; epoch_index++) {
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

	// 		if (epoch_index < 16000) {
	// 			fold_network->backprop(errors, 0.01);
	// 		} else {
	// 			fold_network->backprop(errors, 0.001);
	// 		}
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
	// input_file.open("saves/p2_f_3.txt");
	FoldNetwork* fold_network = new FoldNetwork(input_file);
	input_file.close();
	fold_network->add_scope(1);	// score_state

	double average_score = 0.0;
	double average_error = 0.18;

	vector<Node*> nodes;
	// for (int i = 0; i < 4; i++) {
	// 	ifstream input_file;
	// 	input_file.open("saves/p2_n_" + to_string(i) + "_3.txt");
	// 	nodes.push_back(new Node("p2_n_" + to_string(i), input_file));
	// 	input_file.close();
	// }
	vector<int> scope_sizes;
	scope_sizes.push_back(1);	// first scope is always score_state
	for (int n_index = 0; n_index < (int)nodes.size(); n_index++) {
		nodes[n_index]->get_scope_sizes(scope_sizes);
	}
	int fold_index = (int)nodes.size();

	for (int compress_index = fold_index; compress_index < 10; compress_index++) {
		TestNode* test_node = new TestNode(1.5*average_error,
										   scope_sizes,
										   fold_network);
		while (true) {
			vector<vector<double>> state_vals;
			state_vals.push_back(vector<double>{average_score});
			vector<bool> scopes_on;
			scopes_on.push_back(true);

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

			if (test_node->state == STATE_DONE) {
				break;
			}
		}

		nodes.push_back(new Node("p2_n_"+to_string(fold_index),
								 test_node->score_network,
								 test_node->just_score,
								 test_node->update_existing_scope,
								 1,
								 test_node->state_network,
								 test_node->compress_num_scopes,
								 test_node->compress_sizes,
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

		delete test_node;
	}

	for (int n_index = 0; n_index < (int)nodes.size(); n_index++) {
		delete nodes[n_index];
	}
	delete fold_network;

	cout << "Done" << endl;
}
