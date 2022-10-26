/**
 * 0 - 2: blank
 * 1 - 2: 1 which is shared val
 * 2 - 2: 1 which is 1st branch score modifier
 * 3 - 2: 1 which is 2nd branch score modifier
 * 4 - 2: 1 which is choice
 * 5 - 3: 1 which is 1st branch val, 1 which is 2nd branch val
 * 6 - 2: 1 which is choice
 * 7 - 2: blank
 * - 1st branch:
 *   - 0 - 3: 1 which is val
 * - 2nd branch:
 *   - 0 - 2: 1 which is val
 *   - 1 - 2: blank
 *   - 2 - 3: 1 which is val
 * 0 - 2: blank
 * 1 - 3: 1 which is 1st branch val
 * 2 - 2: 1 which is 2nd branch val
 * 3 - 2: 1 which is shared val
 * 4 - 2: 1 which is shared score modifier
 * 5 - 2: blank
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

	vector<int> second_branch_choice_flat_sizes;
	second_branch_choice_flat_sizes.push_back(2);
	second_branch_choice_flat_sizes.push_back(2);
	second_branch_choice_flat_sizes.push_back(2);
	second_branch_choice_flat_sizes.push_back(2);
	second_branch_choice_flat_sizes.push_back(2);
	second_branch_choice_flat_sizes.push_back(3);
	second_branch_choice_flat_sizes.push_back(2);
	second_branch_choice_flat_sizes.push_back(2);
	// FoldNetwork* second_branch_choice_flat_network = new FoldNetwork(second_branch_choice_flat_sizes);

	// double sum_error = 0.0;
	// for (int iter_index = 1; iter_index < 500000; iter_index++) {
	// 	if (iter_index%10000 == 0) {
	// 		cout << endl;
	// 		cout << iter_index << endl;
	// 		cout << "sum_error: " << sum_error << endl;
	// 		sum_error = 0.0;
	// 	}

	// 	int choice_val = 0;
	// 	int shared_val = 0;
	// 	int second_val = 0;
	// 	double second_score_modifier = 0.0;
	// 	double shared_score_modifier = 0.0;

	// 	vector<vector<double>> flat_vals;
	// 	flat_vals.reserve(8);

	// 	flat_vals.push_back(vector<double>(2));
	// 	flat_vals[0][0] = rand()%2*2-1;
	// 	flat_vals[0][1] = rand()%2*2-1;

	// 	flat_vals.push_back(vector<double>(2));
	// 	flat_vals[1][0] = rand()%2*2-1;
	// 	if (flat_vals[1][0] == 1.0) {
	// 		shared_val++;
	// 	}
	// 	flat_vals[1][1] = rand()%2*2-1;

	// 	flat_vals.push_back(vector<double>(2));
	// 	flat_vals[2][0] = rand()%2*2-1;
	// 	flat_vals[2][1] = rand()%2*2-1;

	// 	flat_vals.push_back(vector<double>(2));
	// 	flat_vals[3][0] = rand()%2*2-1;
	// 	second_score_modifier += flat_vals[3][0];
	// 	flat_vals[3][1] = rand()%2*2-1;

	// 	flat_vals.push_back(vector<double>(2));
	// 	flat_vals[4][0] = rand()%2*2-1;
	// 	if (flat_vals[4][0] == 1.0) {
	// 		choice_val++;
	// 	}
	// 	flat_vals[4][1] = rand()%2*2-1;

	// 	flat_vals.push_back(vector<double>(3));
	// 	flat_vals[5][0] = rand()%2*2-1;
	// 	flat_vals[5][1] = rand()%2*2-1;
	// 	if (flat_vals[5][1] == 1.0) {
	// 		second_val++;
	// 	}
	// 	flat_vals[5][2] = rand()%2*2-1;

	// 	flat_vals.push_back(vector<double>(2));
	// 	flat_vals[6][0] = rand()%2*2-1;
	// 	if (flat_vals[6][0] == 1.0) {
	// 		choice_val++;
	// 	}
	// 	flat_vals[6][1] = rand()%2*2-1;

	// 	flat_vals.push_back(vector<double>(2));
	// 	flat_vals[7][0] = rand()%2*2-1;
	// 	flat_vals[7][1] = rand()%2*2-1;

	// 	double flat_vals_8_0 = rand()%2*2-1;
	// 	if (flat_vals_8_0 == 1.0) {
	// 		second_val++;
	// 	}

	// 	double flat_vals_10_0 = rand()%2*2-1;
	// 	if (flat_vals_10_0 == 1.0) {
	// 		second_val++;
	// 	}

	// 	double flat_vals_13_0 = rand()%2*2-1;
	// 	if (flat_vals_13_0 == 1.0) {
	// 		second_val++;
	// 	}

	// 	double flat_vals_14_0 = rand()%2*2-1;
	// 	if (flat_vals_14_0 == 1.0) {
	// 		shared_val++;
	// 	}

	// 	double flat_vals_15_0 = rand()%2*2-1;
	// 	shared_score_modifier += flat_vals_15_0;

	// 	second_branch_choice_flat_network->activate(flat_vals);

	// 	double final_val;
	// 	if (choice_val%2 == 0) {
	// 		final_val = -5.0;
	// 	} else {
	// 		final_val = (shared_val+second_val)%2 + second_score_modifier + shared_score_modifier;
	// 	}

	// 	vector<double> errors;
	// 	errors.push_back(final_val - second_branch_choice_flat_network->output->acti_vals[0]);
	// 	sum_error += abs(errors[0]);

	// 	if (iter_index < 400000) {
	// 		second_branch_choice_flat_network->backprop(errors, 0.01);
	// 	} else {
	// 		second_branch_choice_flat_network->backprop(errors, 0.001);
	// 	}
	// }

	// ofstream output_file;
	// output_file.open("saves/second_branch_choice_fold_network.txt");
	// second_branch_choice_flat_network->save(output_file);
	// output_file.close();

	ifstream input_file;
	input_file.open("saves/second_branch_choice_fold_network.txt");
	FoldNetwork* second_branch_choice_flat_network = new FoldNetwork(input_file);
	input_file.close();

	vector<Node*> nodes;
	for (int i = 0; i < 15; i++) {
		ifstream input_file;
		input_file.open("saves/n_" + to_string(i) + "_14.txt");
		nodes.push_back(new Node(input_file));
		input_file.close();
	}

	{
		vector<vector<double>> state_vals;
		double predicted_score = 0.0;

		int flat_input_counter = 0;

		int choice_val = 0;
		int shared_val = 0;
		int first_val = 0;
		int second_val = 0;
		double first_score_modifier = 0.0;
		double second_score_modifier = 0.0;
		double shared_score_modifier = 0.0;

		vector<vector<double>> flat_vals;
		flat_vals.reserve(8);

		flat_vals.push_back(vector<double>(2));
		flat_vals[0][0] = rand()%2*2-1;
		flat_vals[0][1] = rand()%2*2-1;
		nodes[flat_input_counter]->activate(state_vals,
											flat_vals[flat_input_counter],
											predicted_score);
		flat_input_counter++;

		flat_vals.push_back(vector<double>(2));
		flat_vals[1][0] = rand()%2*2-1;
		if (flat_vals[1][0] == 1.0) {
			shared_val++;
		}
		flat_vals[1][1] = rand()%2*2-1;
		nodes[flat_input_counter]->activate(state_vals,
											flat_vals[flat_input_counter],
											predicted_score);
		flat_input_counter++;

		flat_vals.push_back(vector<double>(2));
		flat_vals[2][0] = rand()%2*2-1;
		first_score_modifier += flat_vals[2][0];
		flat_vals[2][1] = rand()%2*2-1;
		nodes[flat_input_counter]->activate(state_vals,
											flat_vals[flat_input_counter],
											predicted_score);
		flat_input_counter++;

		flat_vals.push_back(vector<double>(2));
		flat_vals[3][0] = rand()%2*2-1;
		second_score_modifier += flat_vals[3][0];
		flat_vals[3][1] = rand()%2*2-1;
		nodes[flat_input_counter]->activate(state_vals,
											flat_vals[flat_input_counter],
											predicted_score);
		flat_input_counter++;

		flat_vals.push_back(vector<double>(2));
		flat_vals[4][0] = rand()%2*2-1;
		if (flat_vals[4][0] == 1.0) {
			choice_val++;
		}
		flat_vals[4][1] = rand()%2*2-1;
		nodes[flat_input_counter]->activate(state_vals,
											flat_vals[flat_input_counter],
											predicted_score);
		flat_input_counter++;

		flat_vals.push_back(vector<double>(3));
		flat_vals[5][0] = rand()%2*2-1;
		if (flat_vals[5][0] == 1.0) {
			first_val++;
		}
		flat_vals[5][1] = rand()%2*2-1;
		if (flat_vals[5][1] == 1.0) {
			second_val++;
		}
		flat_vals[5][2] = rand()%2*2-1;
		nodes[flat_input_counter]->activate(state_vals,
											flat_vals[flat_input_counter],
											predicted_score);
		flat_input_counter++;

		flat_vals.push_back(vector<double>(2));
		flat_vals[6][0] = rand()%2*2-1;
		if (flat_vals[6][0] == 1.0) {
			choice_val++;
		}
		flat_vals[6][1] = rand()%2*2-1;
		nodes[flat_input_counter]->activate(state_vals,
											flat_vals[flat_input_counter],
											predicted_score);
		flat_input_counter++;

		flat_vals.push_back(vector<double>(2));
		flat_vals[7][0] = rand()%2*2-1;
		flat_vals[7][1] = rand()%2*2-1;
		nodes[flat_input_counter]->activate(state_vals,
											flat_vals[flat_input_counter],
											predicted_score);
		flat_input_counter++;

		second_branch_choice_flat_network->activate(flat_vals);

		cout << "1st branch prediction: " << predicted_score << endl;
		cout << "2nd branch prediction: " << second_branch_choice_flat_network->output->acti_vals[0] << endl;

		double final_val;
		if (predicted_score > second_branch_choice_flat_network->output->acti_vals[0]) {
			// 1st branch
			double flat_vals_8_0 = rand()%2*2-1;
			if (flat_vals_8_0 == 1.0) {
				first_val++;
			}

			double flat_vals_10_0 = rand()%2*2-1;
			if (flat_vals_10_0 == 1.0) {
				first_val++;
			}

			double flat_vals_12_0 = rand()%2*2-1;
			if (flat_vals_12_0 == 1.0) {
				shared_val++;
			}

			double flat_vals_13_0 = rand()%2*2-1;
			shared_score_modifier += flat_vals_13_0;

			if (choice_val%2 == 0) {
				final_val = (shared_val+first_val)%2 + first_score_modifier + shared_score_modifier;
			} else {
				final_val = -5.0;
			}
		} else {
			// 2nd branch
			double flat_vals_8_0 = rand()%2*2-1;
			if (flat_vals_8_0 == 1.0) {
				second_val++;
			}

			double flat_vals_10_0 = rand()%2*2-1;
			if (flat_vals_10_0 == 1.0) {
				second_val++;
			}

			double flat_vals_13_0 = rand()%2*2-1;
			if (flat_vals_13_0 == 1.0) {
				second_val++;
			}

			double flat_vals_14_0 = rand()%2*2-1;
			if (flat_vals_14_0 == 1.0) {
				shared_val++;
			}

			double flat_vals_15_0 = rand()%2*2-1;
			shared_score_modifier += flat_vals_15_0;

			if (choice_val%2 == 0) {
				final_val = -5.0;
			} else {
				final_val = (shared_val+second_val)%2 + second_score_modifier + shared_score_modifier;
			}
		}

		cout << "final_val: " << final_val << endl;
	}

	delete second_branch_choice_flat_network;
	for (int n_index = 0; n_index < (int)nodes.size(); n_index++) {
		delete nodes[n_index];
	}

	cout << "Done" << endl;
}