/**
 * 0 - 2: blank
 * 1 - 2: 1 which is index val
 * 2 - 2: 1 which is val
 * 3 - 2: 1 which is val
 * 4 - 3: 1 which is val
 * 5 - 2: blank
 * 6 - 2: 1 which is clear val
 * 7 - 2: blank
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
double global_sum_error;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	vector<int> initial_flat_sizes;
	initial_flat_sizes.push_back(2);
	initial_flat_sizes.push_back(2);
	initial_flat_sizes.push_back(2);
	initial_flat_sizes.push_back(2);
	initial_flat_sizes.push_back(3);
	initial_flat_sizes.push_back(2);
	initial_flat_sizes.push_back(2);
	initial_flat_sizes.push_back(2);
	// FoldNetwork* initial_fold_network = new FoldNetwork(initial_flat_sizes);

	// double sum_error = 0.0;
	// for (int iter_index = 1; iter_index < 500000; iter_index++) {
	// 	if (iter_index%10000 == 0) {
	// 		cout << endl;
	// 		cout << iter_index << endl;
	// 		cout << "sum_error: " << sum_error << endl;
	// 		sum_error = 0.0;
	// 	}

	// 	double final_val = 0;

	// 	vector<vector<double>> flat_vals;
	// 	flat_vals.reserve(8);

	// 	flat_vals.push_back(vector<double>(2));
	// 	flat_vals[0][0] = rand()%2*2-1;
	// 	flat_vals[0][1] = rand()%2*2-1;

	// 	flat_vals.push_back(vector<double>(2));
	// 	int index = rand()%3;
	// 	flat_vals[1][0] = index;
	// 	flat_vals[1][1] = rand()%2*2-1;

	// 	flat_vals.push_back(vector<double>(2));
	// 	flat_vals[2][0] = rand()%2*2-1;
	// 	if (index == 0) {
	// 		final_val += flat_vals[2][0];
	// 	}
	// 	final_val += flat_vals[2][0];
	// 	flat_vals[2][1] = rand()%2*2-1;

	// 	flat_vals.push_back(vector<double>(2));
	// 	flat_vals[3][0] = rand()%2*2-1;
	// 	if (index == 1) {
	// 		final_val += flat_vals[3][0];
	// 	}
	// 	final_val += flat_vals[3][0];
	// 	flat_vals[3][1] = rand()%2*2-1;

	// 	flat_vals.push_back(vector<double>(3));
	// 	flat_vals[4][0] = rand()%2*2-1;
	// 	if (index == 2) {
	// 		final_val += flat_vals[4][0];
	// 	}
	// 	final_val += flat_vals[4][0];
	// 	flat_vals[4][1] = rand()%2*2-1;
	// 	flat_vals[4][2] = rand()%2*2-1;

	// 	flat_vals.push_back(vector<double>(2));
	// 	flat_vals[5][0] = rand()%2*2-1;
	// 	flat_vals[5][1] = rand()%2*2-1;

	// 	flat_vals.push_back(vector<double>(2));
	// 	flat_vals[6][0] = rand()%2*2-1;
	// 	if (flat_vals[6][0] == 1.0) {
	// 		final_val = 0.0;
	// 	}
	// 	flat_vals[6][1] = rand()%2*2-1;

	// 	flat_vals.push_back(vector<double>(2));
	// 	flat_vals[7][0] = rand()%2*2-1;
	// 	flat_vals[7][1] = rand()%2*2-1;

	// 	initial_fold_network->activate(flat_vals);

	// 	vector<double> errors;
	// 	errors.push_back(final_val - initial_fold_network->output->acti_vals[0]);
	// 	sum_error += abs(errors[0]);

	// 	if (iter_index < 300000) {
	// 		initial_fold_network->backprop(errors, 0.05);
	// 	} else if (iter_index < 400000) {
	// 		initial_fold_network->backprop(errors, 0.01);
	// 	} else {
	// 		initial_fold_network->backprop(errors, 0.001);
	// 	}
	// }

	// ofstream output_file;
	// output_file.open("saves/initial_fold_network.txt");
	// initial_fold_network->save(output_file);
	// output_file.close();

	ifstream input_file;
	input_file.open("saves/initial_fold_network.txt");
	// input_file.open("saves/f_2.txt");
	FoldNetwork* initial_fold_network = new FoldNetwork(input_file);
	input_file.close();

	double average_score = 0.0;
	double max_allowable_error = 0.01;
	double max_needed_error = 0.005;

	vector<Node*> nodes;
	// for (int i = 0; i < 3; i++) {
	// 	ifstream input_file;
	// 	input_file.open("saves/n_" + to_string(i) + "_2.txt");
	// 	nodes.push_back(new Node(input_file));
	// 	input_file.close();
	// }
	vector<int> scope_sizes;
	vector<int> s_input_sizes;
	for (int n_index = 0; n_index < (int)nodes.size(); n_index++) {
		nodes[n_index]->get_scope_sizes(scope_sizes,
										s_input_sizes);
	}
	int fold_index = (int)nodes.size();

	for (int compress_index = fold_index; compress_index < 8; compress_index++) {
		TestNode* test_node = new TestNode(scope_sizes,
										   s_input_sizes,
										   initial_fold_network,
										   initial_flat_sizes[fold_index],
										   max_allowable_error,
										   max_needed_error);
		while (true) {
			vector<vector<double>> state_vals;
			vector<vector<double>> s_input_vals;
			double predicted_score = average_score;

			int flat_input_counter = 0;

			double final_val = 0;

			vector<vector<double>> flat_vals;
			flat_vals.reserve(8);

			flat_vals.push_back(vector<double>(2));
			flat_vals[0][0] = rand()%2*2-1;
			flat_vals[0][1] = rand()%2*2-1;
			if (fold_index > flat_input_counter) {
				nodes[flat_input_counter]->activate(state_vals,
													s_input_vals,
													flat_vals[0],
													predicted_score);
			} else if (fold_index == flat_input_counter) {
				test_node->activate(state_vals,
									s_input_vals,
									flat_vals[0],
									predicted_score);
			}
			flat_input_counter++;

			flat_vals.push_back(vector<double>(2));
			int index = rand()%3;
			flat_vals[1][0] = index;
			flat_vals[1][1] = rand()%2*2-1;
			if (fold_index > flat_input_counter) {
				nodes[flat_input_counter]->activate(state_vals,
													s_input_vals,
													flat_vals[1],
													predicted_score);
			} else if (fold_index == flat_input_counter) {
				test_node->activate(state_vals,
									s_input_vals,
									flat_vals[1],
									predicted_score);
			}
			flat_input_counter++;

			flat_vals.push_back(vector<double>(2));
			flat_vals[2][0] = rand()%2*2-1;
			if (index == 0) {
				final_val += flat_vals[2][0];
			}
			final_val += flat_vals[2][0];
			flat_vals[2][1] = rand()%2*2-1;
			if (fold_index > flat_input_counter) {
				nodes[flat_input_counter]->activate(state_vals,
													s_input_vals,
													flat_vals[2],
													predicted_score);
			} else if (fold_index == flat_input_counter) {
				test_node->activate(state_vals,
									s_input_vals,
									flat_vals[2],
									predicted_score);
			}
			flat_input_counter++;

			flat_vals.push_back(vector<double>(2));
			flat_vals[3][0] = rand()%2*2-1;
			if (index == 1) {
				final_val += flat_vals[3][0];
			}
			final_val += flat_vals[3][0];
			flat_vals[3][1] = rand()%2*2-1;
			if (fold_index > flat_input_counter) {
				nodes[flat_input_counter]->activate(state_vals,
													s_input_vals,
													flat_vals[3],
													predicted_score);
			} else if (fold_index == flat_input_counter) {
				test_node->activate(state_vals,
									s_input_vals,
									flat_vals[3],
									predicted_score);
			}
			flat_input_counter++;

			flat_vals.push_back(vector<double>(3));
			flat_vals[4][0] = rand()%2*2-1;
			if (index == 2) {
				final_val += flat_vals[4][0];
			}
			final_val += flat_vals[4][0];
			flat_vals[4][1] = rand()%2*2-1;
			flat_vals[4][2] = rand()%2*2-1;
			if (fold_index > flat_input_counter) {
				nodes[flat_input_counter]->activate(state_vals,
													s_input_vals,
													flat_vals[4],
													predicted_score);
			} else if (fold_index == flat_input_counter) {
				test_node->activate(state_vals,
									s_input_vals,
									flat_vals[4],
									predicted_score);
			}
			flat_input_counter++;

			flat_vals.push_back(vector<double>(2));
			flat_vals[5][0] = rand()%2*2-1;
			flat_vals[5][1] = rand()%2*2-1;
			if (fold_index > flat_input_counter) {
				nodes[flat_input_counter]->activate(state_vals,
													s_input_vals,
													flat_vals[5],
													predicted_score);
			} else if (fold_index == flat_input_counter) {
				test_node->activate(state_vals,
									s_input_vals,
									flat_vals[5],
									predicted_score);
			}
			flat_input_counter++;

			flat_vals.push_back(vector<double>(2));
			flat_vals[6][0] = rand()%2*2-1;
			if (flat_vals[6][0] == 1.0) {
				final_val = 0.0;
			}
			flat_vals[6][1] = rand()%2*2-1;
			if (fold_index > flat_input_counter) {
				nodes[flat_input_counter]->activate(state_vals,
													s_input_vals,
													flat_vals[6],
													predicted_score);
			} else if (fold_index == flat_input_counter) {
				test_node->activate(state_vals,
									s_input_vals,
									flat_vals[6],
									predicted_score);
			}
			flat_input_counter++;

			flat_vals.push_back(vector<double>(2));
			flat_vals[7][0] = rand()%2*2-1;
			flat_vals[7][1] = rand()%2*2-1;
			if (fold_index > flat_input_counter) {
				nodes[flat_input_counter]->activate(state_vals,
													s_input_vals,
													flat_vals[7],
													predicted_score);
			} else if (fold_index == flat_input_counter) {
				test_node->activate(state_vals,
									s_input_vals,
									flat_vals[7],
									predicted_score);
			}
			flat_input_counter++;

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
								 test_node->obs_size,
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
								 test_node->compressed_scope_sizes,
								 test_node->compressed_s_input_sizes));

		for (int n_index = 0; n_index < (int)nodes.size(); n_index++) {
			ofstream output_file;
			output_file.open("saves/" + nodes[n_index]->id + "_" + to_string(compress_index) + ".txt");
			nodes[n_index]->save(output_file);
			output_file.close();
		}

		delete initial_fold_network;
		initial_fold_network = test_node->curr_fold;

		{
			ofstream output_file;
			output_file.open("saves/f_" + to_string(compress_index) + ".txt");
			initial_fold_network->save(output_file);
			output_file.close();
		}

		cout << "SAVED " << nodes.back()->id << endl;

		scope_sizes = test_node->curr_scope_sizes;
		s_input_sizes = test_node->curr_s_input_sizes;

		fold_index++;

		delete test_node;
	}

	for (int n_index = 0; n_index < (int)nodes.size(); n_index++) {
		delete nodes[n_index];
	}
	delete initial_fold_network;

	cout << "Done" << endl;
}
