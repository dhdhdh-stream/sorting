/**
 * 0 - 2: blank
 * 1 - 2: 1 which is shared val
 * 2 - 2: 1 which is shared score modifier
 * 3 - 2: 1 which is choice
 * 4 - 3: 1 which is shared val
 * 5 - 2: 1 which is choice
 * - 1st branch:
 *   - 0 - 3: 1 which is val
 * - 2nd branch:
 *   - 0 - 2: 1 which is val
 *   - 1 - 2: blank
 *   - 2 - 3: 1 which is val
 * 0 - 2: blank
 * 1 - 2: 1 which is shared score modifier
 * 2 - 2: 1 which is shared val
 * 3 - 2: blank
 */

#include <chrono>
#include <iostream>
#include <thread>
#include <random>

#include "fold_network.h"
#include "network.h"
#include "node.h"
#include "scope.h"
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

	vector<Node*> nodes;
	for (int i = 0; i < 11; i++) {
		ifstream input_file;
		input_file.open("saves/n_" + to_string(i) + "_10.txt");
		nodes.push_back(new Node(input_file));
		input_file.close();
	}

	Scope* scope = construct_scope(nodes);

	vector<int> explore_path;
	explore_path.push_back(1);
	int explore_start_inclusive = 2;
	int explore_end_non_inclusive = 3;

	Network* front_network = new Network(11, 200, 1);
	Network* back_network = new Network(11, 100, 1);

	global_sum_error = 0.0;
	for (int iter_index = 1; iter_index < 1000000; iter_index++) {
		if (iter_index%10000 == 0) {
			cout << endl;
			cout << iter_index << endl;
			cout << "sum_error: " << global_sum_error << endl;
			global_sum_error = 0.0;
		}

		int choice_val = 0;
		int xor_val = 0;
		double score_modifier = 0.0;

		vector<vector<double>> flat_vals;
		flat_vals.reserve(13);

		flat_vals.push_back(vector<double>(2));
		flat_vals[0][0] = rand()%2*2-1;
		flat_vals[0][1] = rand()%2*2-1;

		flat_vals.push_back(vector<double>(2));
		flat_vals[1][0] = rand()%2*2-1;
		if (flat_vals[1][0] == 1.0) {
			xor_val++;
		}
		flat_vals[1][1] = rand()%2*2-1;

		flat_vals.push_back(vector<double>(2));
		flat_vals[2][0] = rand()%2*2-1;
		score_modifier += flat_vals[2][0];
		flat_vals[2][1] = rand()%2*2-1;

		flat_vals.push_back(vector<double>(2));
		flat_vals[3][0] = rand()%2*2-1;
		if (flat_vals[3][0] == 1.0) {
			choice_val++;
		}
		flat_vals[3][1] = rand()%2*2-1;

		flat_vals.push_back(vector<double>(3));
		flat_vals[4][0] = rand()%2*2-1;
		if (flat_vals[4][0] == 1.0) {
			xor_val++;
		}
		flat_vals[4][1] = rand()%2*2-1;
		flat_vals[4][2] = rand()%2*2-1;

		flat_vals.push_back(vector<double>(2));
		flat_vals[5][0] = rand()%2*2-1;
		if (flat_vals[5][0] == 1.0) {
			choice_val++;
		}
		flat_vals[5][1] = rand()%2*2-1;

		flat_vals.push_back(vector<double>(2));
		flat_vals[6][0] = rand()%2*2-1;
		if (flat_vals[6][0] == 1.0) {
			xor_val++;
		}
		flat_vals[6][1] = rand()%2*2-1;

		flat_vals.push_back(vector<double>(2));
		flat_vals[7][0] = rand()%2*2-1;
		flat_vals[7][1] = rand()%2*2-1;

		flat_vals.push_back(vector<double>(3));
		flat_vals[8][0] = rand()%2*2-1;
		if (flat_vals[8][0] == 1.0) {
			xor_val++;
		}
		flat_vals[8][1] = rand()%2*2-1;
		flat_vals[8][2] = rand()%2*2-1;

		flat_vals.push_back(vector<double>(2));
		flat_vals[9][0] = rand()%2*2-1;
		flat_vals[9][1] = rand()%2*2-1;

		flat_vals.push_back(vector<double>(3));
		flat_vals[10][0] = rand()%2*2-1;
		score_modifier += flat_vals[10][0];
		flat_vals[10][1] = rand()%2*2-1;

		flat_vals.push_back(vector<double>(2));
		flat_vals[11][0] = rand()%2*2-1;
		if (flat_vals[11][0] == 1.0) {
			xor_val++;
		}
		flat_vals[11][1] = rand()%2*2-1;

		flat_vals.push_back(vector<double>(2));
		flat_vals[12][0] = rand()%2*2-1;
		flat_vals[12][1] = rand()%2*2-1;

		double final_val;
		if (choice_val%2 == 0) {
			final_val = -5.0;
		} else {
			final_val = xor_val%2 + score_modifier;
		}

		vector<double> inputs;	// empty
		double front_predicted_score = 0.0;	// initialization doesn't matter
		double back_predicted_score = 0.0;	// initialization doesn't matter
		scope->branch_combine_activate(flat_vals,
									   inputs,
									   explore_path,
									   explore_start_inclusive,
									   explore_end_non_inclusive,
									   front_network,
									   back_network,
									   front_predicted_score,
									   back_predicted_score);

		double predicted_score = front_predicted_score + back_predicted_score;

		global_sum_error += abs(final_val - predicted_score);
		if (iter_index%10000 == 0) {
			cout << "final_val: " << final_val << endl;
			cout << "predicted_score: " << predicted_score << endl;
		}

		double front_error = (final_val - back_predicted_score) - front_predicted_score;

		vector<double> input_errors;	// empty
		double target_max_update;
		if (iter_index < 200000) {
			target_max_update = 0.05;
		} else if (iter_index < 800000) {
			target_max_update = 0.01;
		} else {
			target_max_update = 0.001;
		}
		// if (iter_index < 800000) {
		// 	target_max_update = 0.01;
		// } else {
		// 	target_max_update = 0.001;
		// }
		scope->branch_combine_backprop(input_errors,
									   predicted_score,
									   final_val,
									   explore_path,
									   explore_start_inclusive,
									   explore_end_non_inclusive,
									   front_network,
									   front_error,
									   back_network,
									   target_max_update);
	}

	for (int n_index = 0; n_index < (int)nodes.size(); n_index++) {
		delete nodes[n_index];
	}
	delete scope;
	delete front_network;
	delete back_network;

	cout << "Done" << endl;
}
