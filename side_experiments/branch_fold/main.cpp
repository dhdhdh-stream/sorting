/**
 * 0 - 2: blank
 * 1 - 3: 2 which are 1st xor
 * 2 - 2: 1 which is 1st xor
 * 3 - 3: blank
 * 4 - 4: 1 which is 1st xor
 * 5 - 1: blank
 * 6 - 3: 1 which is 2nd xor
 * 7 - 2: 2 which is 2nd xor
 * 8 - 3: blank
 * 9 - 2: 1 which is 2nd xor
 * 10 - 2: blank
 */

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

	vector<int> flat_sizes;
	flat_sizes.push_back(2);
	flat_sizes.push_back(3);
	flat_sizes.push_back(2);
	flat_sizes.push_back(3);
	flat_sizes.push_back(4);
	flat_sizes.push_back(1);
	flat_sizes.push_back(3);
	flat_sizes.push_back(2);
	flat_sizes.push_back(3);
	flat_sizes.push_back(2);
	flat_sizes.push_back(2);
	// FoldNetwork* fold_network = new FoldNetwork(flat_sizes);

	// double sum_error = 0.0;
	// for (int epoch_index = 1; epoch_index < 5000; epoch_index++) {
	// 	if (epoch_index%100 == 0) {
	// 		cout << endl;
	// 		cout << epoch_index << endl;
	// 		cout << "sum_error: " << sum_error << endl;
	// 		sum_error = 0.0;
	// 	}

	// 	for (int iter_index = 0; iter_index < 100; iter_index++) {
	// 		vector<vector<double>> flat_vals;
	// 		flat_vals.reserve(11);

	// 		flat_vals.push_back(vector<double>(2));
	// 		flat_vals[0][0] = rand()%2*2-1;
	// 		flat_vals[0][1] = rand()%2*2-1;

	// 		int first_sum = 0;

	// 		flat_vals.push_back(vector<double>(3));
	// 		flat_vals[1][0] = rand()%2*2-1;
	// 		if (flat_vals[1][0] == 1.0) {
	// 			first_sum++;
	// 		}
	// 		flat_vals[1][1] = rand()%2*2-1;
	// 		if (flat_vals[1][1] == 1.0) {
	// 			first_sum++;
	// 		}
	// 		flat_vals[1][2] = rand()%2*2-1;

	// 		flat_vals.push_back(vector<double>(2));
	// 		flat_vals[2][0] = rand()%2*2-1;
	// 		if (flat_vals[2][0] == 1.0) {
	// 			first_sum++;
	// 		}
	// 		flat_vals[2][1] = rand()%2*2-1;

	// 		flat_vals.push_back(vector<double>(3));
	// 		flat_vals[3][0] = rand()%2*2-1;
	// 		flat_vals[3][1] = rand()%2*2-1;
	// 		flat_vals[3][2] = rand()%2*2-1;

	// 		flat_vals.push_back(vector<double>(4));
	// 		flat_vals[4][0] = rand()%2*2-1;
	// 		if (flat_vals[4][0] == 1.0) {
	// 			first_sum++;
	// 		}
	// 		flat_vals[4][1] = rand()%2*2-1;
	// 		flat_vals[4][2] = rand()%2*2-1;
	// 		flat_vals[4][3] = rand()%2*2-1;

	// 		flat_vals.push_back(vector<double>(1));
	// 		flat_vals[5][0] = rand()%2*2-1;

	// 		int second_sum = 0;

	// 		flat_vals.push_back(vector<double>(3));
	// 		flat_vals[6][0] = rand()%2*2-1;
	// 		if (flat_vals[6][0] == 1.0) {
	// 			second_sum++;
	// 		}
	// 		flat_vals[6][1] = rand()%2*2-1;
	// 		flat_vals[6][2] = rand()%2*2-1;

	// 		flat_vals.push_back(vector<double>(2));
	// 		flat_vals[7][0] = rand()%2*2-1;
	// 		if (flat_vals[7][0] == 1.0) {
	// 			second_sum++;
	// 		}
	// 		flat_vals[7][1] = rand()%2*2-1;
	// 		if (flat_vals[7][1] == 1.0) {
	// 			second_sum++;
	// 		}

	// 		flat_vals.push_back(vector<double>(3));
	// 		flat_vals[8][0] = rand()%2*2-1;
	// 		flat_vals[8][1] = rand()%2*2-1;
	// 		flat_vals[8][2] = rand()%2*2-1;

	// 		flat_vals.push_back(vector<double>(2));
	// 		flat_vals[9][0] = rand()%2*2-1;
	// 		if (flat_vals[9][0] == 1.0) {
	// 			second_sum++;
	// 		}
	// 		flat_vals[9][1] = rand()%2*2-1;

	// 		flat_vals.push_back(vector<double>(2));
	// 		flat_vals[10][0] = rand()%2*2-1;
	// 		flat_vals[10][1] = rand()%2*2-1;

	// 		fold_network->activate(flat_vals);

	// 		double final_val = first_sum%2 - second_sum%2;

	// 		vector<double> errors;
	// 		errors.push_back(final_val - fold_network->output->acti_vals[0]);
	// 		sum_error += abs(errors[0]);

	// 		if (epoch_index < 4000) {
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
	// 	vector<vector<double>> flat_vals;
	// 	flat_vals.reserve(11);

	// 	flat_vals.push_back(vector<double>(2));
	// 	flat_vals[0][0] = rand()%2*2-1;
	// 	flat_vals[0][1] = rand()%2*2-1;

	// 	int first_sum = 0;

	// 	flat_vals.push_back(vector<double>(3));
	// 	flat_vals[1][0] = rand()%2*2-1;
	// 	if (flat_vals[1][0] == 1.0) {
	// 		first_sum++;
	// 	}
	// 	flat_vals[1][1] = rand()%2*2-1;
	// 	if (flat_vals[1][1] == 1.0) {
	// 		first_sum++;
	// 	}
	// 	flat_vals[1][2] = rand()%2*2-1;

	// 	flat_vals.push_back(vector<double>(2));
	// 	flat_vals[2][0] = rand()%2*2-1;
	// 	if (flat_vals[2][0] == 1.0) {
	// 		first_sum++;
	// 	}
	// 	flat_vals[2][1] = rand()%2*2-1;

	// 	flat_vals.push_back(vector<double>(3));
	// 	flat_vals[3][0] = rand()%2*2-1;
	// 	flat_vals[3][1] = rand()%2*2-1;
	// 	flat_vals[3][2] = rand()%2*2-1;

	// 	flat_vals.push_back(vector<double>(4));
	// 	flat_vals[4][0] = rand()%2*2-1;
	// 	if (flat_vals[4][0] == 1.0) {
	// 		first_sum++;
	// 	}
	// 	flat_vals[4][1] = rand()%2*2-1;
	// 	flat_vals[4][2] = rand()%2*2-1;
	// 	flat_vals[4][3] = rand()%2*2-1;

	// 	flat_vals.push_back(vector<double>(1));
	// 	flat_vals[5][0] = rand()%2*2-1;

	// 	int second_sum = 0;

	// 	flat_vals.push_back(vector<double>(3));
	// 	flat_vals[6][0] = rand()%2*2-1;
	// 	if (flat_vals[6][0] == 1.0) {
	// 		second_sum++;
	// 	}
	// 	flat_vals[6][1] = rand()%2*2-1;
	// 	flat_vals[6][2] = rand()%2*2-1;

	// 	flat_vals.push_back(vector<double>(2));
	// 	flat_vals[7][0] = rand()%2*2-1;
	// 	if (flat_vals[7][0] == 1.0) {
	// 		second_sum++;
	// 	}
	// 	flat_vals[7][1] = rand()%2*2-1;
	// 	if (flat_vals[7][1] == 1.0) {
	// 		second_sum++;
	// 	}

	// 	flat_vals.push_back(vector<double>(3));
	// 	flat_vals[8][0] = rand()%2*2-1;
	// 	flat_vals[8][1] = rand()%2*2-1;
	// 	flat_vals[8][2] = rand()%2*2-1;

	// 	flat_vals.push_back(vector<double>(2));
	// 	flat_vals[9][0] = rand()%2*2-1;
	// 	if (flat_vals[9][0] == 1.0) {
	// 		second_sum++;
	// 	}
	// 	flat_vals[9][1] = rand()%2*2-1;

	// 	flat_vals.push_back(vector<double>(2));
	// 	flat_vals[10][0] = rand()%2*2-1;
	// 	flat_vals[10][1] = rand()%2*2-1;

	// 	fold_network->activate(flat_vals);

	// 	double final_val = first_sum%2 - second_sum%2;

	// 	average_error += abs(final_val - fold_network->output->acti_vals[0]);
	// }

	// average_error /= 10000;
	// cout << "average_error: " << average_error << endl;

	ifstream input_file;
	input_file.open("saves/fold_network.txt");
	// input_file.open("saves/f_1.txt");
	FoldNetwork* fold_network = new FoldNetwork(input_file);
	input_file.close();
	fold_network->average_error = 0.01;

	double average_score = 0.0;

	vector<Node*> nodes;
	// for (int i = 0; i < 2; i++) {
	// 	ifstream input_file;
	// 	input_file.open("saves/n_" + to_string(i) + "_1.txt");
	// 	nodes.push_back(new Node(input_file));
	// 	input_file.close();
	// }
	vector<int> scope_sizes;
	for (int n_index = 0; n_index < (int)nodes.size(); n_index++) {
		nodes[n_index]->get_scope_sizes(scope_sizes);
	}
	int fold_index = (int)nodes.size();

	for (int compress_index = fold_index; compress_index < 11; compress_index++) {
		TestNode* test_node = new TestNode(scope_sizes,
										   fold_network,
										   flat_sizes[fold_index]);
		while (true) {
			vector<vector<double>> state_vals;
			double predicted_score = average_score;

			int flat_input_counter = 0;

			vector<vector<double>> flat_vals;
			flat_vals.reserve(11);

			flat_vals.push_back(vector<double>(2));
			flat_vals[0][0] = rand()%2*2-1;
			flat_vals[0][1] = rand()%2*2-1;
			if (fold_index > flat_input_counter) {
				nodes[flat_input_counter]->activate(state_vals,
													flat_vals[0],
													predicted_score);
			} else if (fold_index == flat_input_counter) {
				test_node->activate(state_vals,
									flat_vals[0],
									predicted_score);
			}
			flat_input_counter++;

			int first_sum = 0;

			flat_vals.push_back(vector<double>(3));
			flat_vals[1][0] = rand()%2*2-1;
			if (flat_vals[1][0] == 1.0) {
				first_sum++;
			}
			flat_vals[1][1] = rand()%2*2-1;
			if (flat_vals[1][1] == 1.0) {
				first_sum++;
			}
			flat_vals[1][2] = rand()%2*2-1;
			if (fold_index > flat_input_counter) {
				nodes[flat_input_counter]->activate(state_vals,
													flat_vals[1],
													predicted_score);
			} else if (fold_index == flat_input_counter) {
				test_node->activate(state_vals,
									flat_vals[1],
									predicted_score);
			}
			flat_input_counter++;

			flat_vals.push_back(vector<double>(2));
			flat_vals[2][0] = rand()%2*2-1;
			if (flat_vals[2][0] == 1.0) {
				first_sum++;
			}
			flat_vals[2][1] = rand()%2*2-1;
			if (fold_index > flat_input_counter) {
				nodes[flat_input_counter]->activate(state_vals,
													flat_vals[2],
													predicted_score);
			} else if (fold_index == flat_input_counter) {
				test_node->activate(state_vals,
									flat_vals[2],
									predicted_score);
			}
			flat_input_counter++;

			flat_vals.push_back(vector<double>(3));
			flat_vals[3][0] = rand()%2*2-1;
			flat_vals[3][1] = rand()%2*2-1;
			flat_vals[3][2] = rand()%2*2-1;
			if (fold_index > flat_input_counter) {
				nodes[flat_input_counter]->activate(state_vals,
													flat_vals[3],
													predicted_score);
			} else if (fold_index == flat_input_counter) {
				test_node->activate(state_vals,
									flat_vals[3],
									predicted_score);
			}
			flat_input_counter++;

			flat_vals.push_back(vector<double>(4));
			flat_vals[4][0] = rand()%2*2-1;
			if (flat_vals[4][0] == 1.0) {
				first_sum++;
			}
			flat_vals[4][1] = rand()%2*2-1;
			flat_vals[4][2] = rand()%2*2-1;
			flat_vals[4][3] = rand()%2*2-1;
			if (fold_index > flat_input_counter) {
				nodes[flat_input_counter]->activate(state_vals,
													flat_vals[4],
													predicted_score);
			} else if (fold_index == flat_input_counter) {
				test_node->activate(state_vals,
									flat_vals[4],
									predicted_score);
			}
			flat_input_counter++;

			flat_vals.push_back(vector<double>(1));
			flat_vals[5][0] = rand()%2*2-1;
			if (fold_index > flat_input_counter) {
				nodes[flat_input_counter]->activate(state_vals,
													flat_vals[5],
													predicted_score);
			} else if (fold_index == flat_input_counter) {
				test_node->activate(state_vals,
									flat_vals[5],
									predicted_score);
			}
			flat_input_counter++;

			int second_sum = 0;

			flat_vals.push_back(vector<double>(3));
			flat_vals[6][0] = rand()%2*2-1;
			if (flat_vals[6][0] == 1.0) {
				second_sum++;
			}
			flat_vals[6][1] = rand()%2*2-1;
			flat_vals[6][2] = rand()%2*2-1;
			if (fold_index > flat_input_counter) {
				nodes[flat_input_counter]->activate(state_vals,
													flat_vals[6],
													predicted_score);
			} else if (fold_index == flat_input_counter) {
				test_node->activate(state_vals,
									flat_vals[6],
									predicted_score);
			}
			flat_input_counter++;

			flat_vals.push_back(vector<double>(2));
			flat_vals[7][0] = rand()%2*2-1;
			if (flat_vals[7][0] == 1.0) {
				second_sum++;
			}
			flat_vals[7][1] = rand()%2*2-1;
			if (flat_vals[7][1] == 1.0) {
				second_sum++;
			}
			if (fold_index > flat_input_counter) {
				nodes[flat_input_counter]->activate(state_vals,
													flat_vals[7],
													predicted_score);
			} else if (fold_index == flat_input_counter) {
				test_node->activate(state_vals,
									flat_vals[7],
									predicted_score);
			}
			flat_input_counter++;

			flat_vals.push_back(vector<double>(3));
			flat_vals[8][0] = rand()%2*2-1;
			flat_vals[8][1] = rand()%2*2-1;
			flat_vals[8][2] = rand()%2*2-1;
			if (fold_index > flat_input_counter) {
				nodes[flat_input_counter]->activate(state_vals,
													flat_vals[8],
													predicted_score);
			} else if (fold_index == flat_input_counter) {
				test_node->activate(state_vals,
									flat_vals[8],
									predicted_score);
			}
			flat_input_counter++;

			flat_vals.push_back(vector<double>(2));
			flat_vals[9][0] = rand()%2*2-1;
			if (flat_vals[9][0] == 1.0) {
				second_sum++;
			}
			flat_vals[9][1] = rand()%2*2-1;
			if (fold_index > flat_input_counter) {
				nodes[flat_input_counter]->activate(state_vals,
													flat_vals[9],
													predicted_score);
			} else if (fold_index == flat_input_counter) {
				test_node->activate(state_vals,
									flat_vals[9],
									predicted_score);
			}
			flat_input_counter++;

			flat_vals.push_back(vector<double>(2));
			flat_vals[10][0] = rand()%2*2-1;
			flat_vals[10][1] = rand()%2*2-1;
			if (fold_index > flat_input_counter) {
				nodes[flat_input_counter]->activate(state_vals,
													flat_vals[10],
													predicted_score);
			} else if (fold_index == flat_input_counter) {
				test_node->activate(state_vals,
									flat_vals[10],
									predicted_score);
			}
			flat_input_counter++;

			double final_val = first_sum%2 - second_sum%2;

			test_node->process(flat_vals,
							   state_vals,
							   predicted_score,
							   final_val,
							   nodes);

			if (test_node->state == STATE_DONE) {
				break;
			}
		}

		nodes.push_back(new Node("n_"+to_string(fold_index),
								 test_node->new_layer_size,
								 test_node->obs_network,
								 test_node->score_input_layer,
								 test_node->score_input_sizes,
								 test_node->score_input_networks,
								 test_node->small_score_network,
								 test_node->compress_num_layers,
								 test_node->compress_new_size,
								 test_node->input_layer,
								 test_node->input_sizes,
								 test_node->input_networks,
								 test_node->small_compression_network,
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
			output_file.open("saves/f_" + to_string(compress_index) + ".txt");
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
