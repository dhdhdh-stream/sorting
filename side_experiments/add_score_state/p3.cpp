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

	// FoldNetwork* fold_network = new FoldNetwork(11, 1);

	// double sum_error = 0.0;
	// for (int epoch_index = 1; epoch_index < 50000; epoch_index++) {
	// 	if (epoch_index%100 == 0) {
	// 		cout << endl;
	// 		cout << epoch_index << endl;
	// 		cout << "sum_error: " << sum_error << endl;
	// 		sum_error = 0.0;
	// 	}

	// 	for (int iter_index = 0; iter_index < 100; iter_index++) {
	// 		int num_iters = rand()%6;

	// 		double flat_inputs[11];
	// 		int flat_input_counter = 0;
	// 		bool activated[11];

	// 		int rand_non_empty = rand()%6;
	// 		flat_inputs[flat_input_counter] = rand_non_empty;
	// 		activated[flat_input_counter] = true;
	// 		flat_input_counter++;

	// 		int sum = 0;
	// 		for (int i = 0; i < 5; i++) {
	// 			int first_value = rand()%2;
	// 			if (i < num_iters) {
	// 				flat_inputs[flat_input_counter] = first_value;
	// 				activated[flat_input_counter] = true;
	// 			} else {
	// 				flat_inputs[flat_input_counter] = 0.0;
	// 				activated[flat_input_counter] = false;
	// 			}
	// 			flat_input_counter++;

	// 			int second_value = rand()%2;
	// 			if (i < num_iters) {
	// 				flat_inputs[flat_input_counter] = second_value;
	// 				activated[flat_input_counter] = true;
	// 			} else {
	// 				flat_inputs[flat_input_counter] = 0.0;
	// 				activated[flat_input_counter] = false;
	// 			}
	// 			flat_input_counter++;

	// 			if (i < rand_non_empty) {
	// 				if (first_value == second_value) {
	// 					sum += 2;
	// 				}
	// 			} else {
	// 				if (i < num_iters) {
	// 					sum -= 1;
	// 				}
	// 			}
	// 		}

	// 		vector<double> obs;
	// 		obs.push_back(rand()%2);

	// 		fold_network->activate(flat_inputs,
	// 							   activated,
	// 							   obs);

	// 		vector<double> errors;
	// 		errors.push_back(sum - fold_network->output->acti_vals[0]);
	// 		sum_error += abs(errors[0]);

	// 		if (epoch_index < 40000) {
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
	// // for (int iter_index = 0; iter_index < 1; iter_index++) {
	// 	int num_iters = rand()%6;
	// 	// cout << "num_iters: " << num_iters << endl;

	// 	double flat_inputs[6];
	// 	int flat_input_counter = 0;
	// 	bool activated[6];

	// 	int rand_non_empty = rand()%6;
	// 	flat_inputs[flat_input_counter] = rand_non_empty;
	// 	activated[flat_input_counter] = true;
	// 	flat_input_counter++;
	// 	// cout << "rand_non_empty: " << rand_non_empty << endl;

	// 	int sum = 0;
	// 	for (int i = 0; i < 5; i++) {
	// 		int first_value = rand()%2;
	// 		if (i < num_iters) {
	// 			flat_inputs[flat_input_counter] = first_value;
	// 			activated[flat_input_counter] = true;
	// 		} else {
	// 			flat_inputs[flat_input_counter] = 0.0;
	// 			activated[flat_input_counter] = false;
	// 		}
	// 		flat_input_counter++;
	// 		// cout << i << " first_value: " << first_value << endl;

	// 		int second_value = rand()%2;
	// 		if (i < num_iters) {
	// 			flat_inputs[flat_input_counter] = second_value;
	// 			activated[flat_input_counter] = true;
	// 		} else {
	// 			flat_inputs[flat_input_counter] = 0.0;
	// 			activated[flat_input_counter] = false;
	// 		}
	// 		flat_input_counter++;
	// 		// cout << i << " second_value: " << second_value << endl;

	// 		if (i < rand_non_empty) {
	// 			if (first_value == second_value) {
	// 				sum += 2;
	// 			}
	// 		} else {
	// 			if (i < num_iters) {
	// 				sum -= 1;
	// 			}
	// 		}
	// 	}

	// 	vector<double> obs;
	// 	obs.push_back(rand()%2);

	// 	fold_network->activate(flat_inputs,
	// 						   activated,
	// 						   obs);

	// 	average_error += abs(sum - fold_network->output->acti_vals[0]);

	// 	// cout << "target: " << sum << endl;
	// 	// cout << "actual: " << fold_network->output->acti_vals[0] << endl;
	// }

	// average_error /= 10000;
	// cout << "average_error: " << average_error << endl;

	ifstream input_file;
	input_file.open("saves/fold_network.txt");
	// input_file.open("saves/p2_f_3.txt");
	FoldNetwork* fold_network = new FoldNetwork(input_file);
	input_file.close();
	fold_network->add_scope(1);	// score_state
	fold_network->average_error = 0.6;

	double average_score = 0.0;

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

	for (int compress_index = fold_index; compress_index < 11; compress_index++) {
		TestNode* test_node = new TestNode(scope_sizes,
										   fold_network);
		while (true) {
			vector<vector<double>> state_vals;
			state_vals.push_back(vector<double>{average_score});
			vector<bool> scopes_on;
			scopes_on.push_back(true);

			int num_iters = rand()%6;

			double flat_inputs[11];
			int flat_input_counter = 0;
			bool activated[11];

			// first block
			int rand_non_empty = rand()%6;
			flat_inputs[flat_input_counter] = rand_non_empty;
			activated[flat_input_counter] = true;
			if (fold_index > flat_input_counter) {
				nodes[flat_input_counter]->activate(state_vals,
													scopes_on,
													flat_inputs[flat_input_counter],
													activated[flat_input_counter]);
			} else if (fold_index == flat_input_counter) {
				test_node->activate(state_vals,
									scopes_on,
									flat_inputs[flat_input_counter],
									activated[flat_input_counter]);
			}
			flat_input_counter++;

			int sum = 0;
			for (int i = 0; i < 5; i++) {
				int first_value = rand()%2;
				if (i < num_iters) {
					flat_inputs[flat_input_counter] = first_value;
					activated[flat_input_counter] = true;
				} else {
					flat_inputs[flat_input_counter] = 0.0;
					activated[flat_input_counter] = false;
				}
				if (fold_index > flat_input_counter) {
					nodes[flat_input_counter]->activate(state_vals,
														scopes_on,
														flat_inputs[flat_input_counter],
														activated[flat_input_counter]);
				} else if (fold_index == flat_input_counter) {
					test_node->activate(state_vals,
										scopes_on,
										flat_inputs[flat_input_counter],
										activated[flat_input_counter]);
				}
				flat_input_counter++;

				int second_value = rand()%2;
				if (i < num_iters) {
					flat_inputs[flat_input_counter] = second_value;
					activated[flat_input_counter] = true;
				} else {
					flat_inputs[flat_input_counter] = 0.0;
					activated[flat_input_counter] = false;
				}
				if (fold_index > flat_input_counter) {
					nodes[flat_input_counter]->activate(state_vals,
														scopes_on,
														flat_inputs[flat_input_counter],
														activated[flat_input_counter]);
				} else if (fold_index == flat_input_counter) {
					test_node->activate(state_vals,
										scopes_on,
										flat_inputs[flat_input_counter],
										activated[flat_input_counter]);
				}
				flat_input_counter++;

				if (i < rand_non_empty) {
					if (first_value == second_value) {
						sum += 2;
					}
				} else {
					if (i < num_iters) {
						sum -= 1;
					}
				}
			}

			test_node->process(flat_inputs,
							   activated,
							   state_vals,
							   rand()%2,
							   sum,
							   nodes);

			if (test_node->state == STATE_DONE) {
				break;
			}
		}

		nodes.push_back(new Node("p3_n_"+to_string(fold_index),
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
			output_file.open("saves/p3_f_" + to_string(compress_index) + ".txt");
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
