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
 * 1 - 3: 1 which is 1st branch score modifier
 * 2 - 2: 1 which is 2nd branch score modifier
 * 3 - 2: 1 which is shared val
 * 4 - 2: 1 which is shared score modifier
 * 5 - 2: blank
 */

#include <chrono>
#include <iostream>
#include <thread>
#include <random>

#include "branch_fold_network.h"
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

	vector<int> pre_branch_flat_sizes;
	pre_branch_flat_sizes.push_back(2);
	pre_branch_flat_sizes.push_back(2);
	pre_branch_flat_sizes.push_back(2);
	pre_branch_flat_sizes.push_back(2);
	pre_branch_flat_sizes.push_back(2);
	pre_branch_flat_sizes.push_back(3);
	pre_branch_flat_sizes.push_back(2);
	pre_branch_flat_sizes.push_back(2);

	vector<int> new_branch_flat_sizes;
	new_branch_flat_sizes.push_back(2);
	new_branch_flat_sizes.push_back(2);
	new_branch_flat_sizes.push_back(3);

	vector<int> post_branch_flat_sizes;
	post_branch_flat_sizes.push_back(2);
	post_branch_flat_sizes.push_back(3);
	post_branch_flat_sizes.push_back(2);
	post_branch_flat_sizes.push_back(2);
	post_branch_flat_sizes.push_back(2);
	post_branch_flat_sizes.push_back(2);
	BranchFoldNetwork* second_branch_fold_network = 
		new BranchFoldNetwork(pre_branch_flat_sizes,
							  new_branch_flat_sizes,
							  post_branch_flat_sizes);

	double sum_error = 0.0;
	for (int iter_index = 1; iter_index < 5000000; iter_index++) {
		if (iter_index%10000 == 0) {
			cout << endl;
			cout << iter_index << endl;
			cout << "sum_error: " << sum_error << endl;
			sum_error = 0.0;
		}

		int choice_val = 0;
		int shared_val = 0;
		int second_val = 0;
		double second_score_modifier = 0.0;
		double shared_score_modifier = 0.0;

		vector<vector<double>> pre_branch_flat_vals;
		pre_branch_flat_vals.reserve(8);

		pre_branch_flat_vals.push_back(vector<double>(2));
		pre_branch_flat_vals[0][0] = rand()%2*2-1;
		pre_branch_flat_vals[0][1] = rand()%2*2-1;

		pre_branch_flat_vals.push_back(vector<double>(2));
		pre_branch_flat_vals[1][0] = rand()%2*2-1;
		if (pre_branch_flat_vals[1][0] == 1.0) {
			shared_val++;
		}
		pre_branch_flat_vals[1][1] = rand()%2*2-1;

		pre_branch_flat_vals.push_back(vector<double>(2));
		pre_branch_flat_vals[2][0] = rand()%2*2-1;
		pre_branch_flat_vals[2][1] = rand()%2*2-1;

		pre_branch_flat_vals.push_back(vector<double>(2));
		pre_branch_flat_vals[3][0] = rand()%2*2-1;
		second_score_modifier += pre_branch_flat_vals[3][0];
		pre_branch_flat_vals[3][1] = rand()%2*2-1;

		pre_branch_flat_vals.push_back(vector<double>(2));
		pre_branch_flat_vals[4][0] = rand()%2*2-1;
		if (pre_branch_flat_vals[4][0] == 1.0) {
			choice_val++;
		}
		pre_branch_flat_vals[4][1] = rand()%2*2-1;

		pre_branch_flat_vals.push_back(vector<double>(3));
		pre_branch_flat_vals[5][0] = rand()%2*2-1;
		pre_branch_flat_vals[5][1] = rand()%2*2-1;
		if (pre_branch_flat_vals[5][1] == 1.0) {
			second_val++;
		}
		pre_branch_flat_vals[5][2] = rand()%2*2-1;

		pre_branch_flat_vals.push_back(vector<double>(2));
		pre_branch_flat_vals[6][0] = rand()%2*2-1;
		if (pre_branch_flat_vals[6][0] == 1.0) {
			choice_val++;
		}
		pre_branch_flat_vals[6][1] = rand()%2*2-1;

		pre_branch_flat_vals.push_back(vector<double>(2));
		pre_branch_flat_vals[7][0] = rand()%2*2-1;
		pre_branch_flat_vals[7][1] = rand()%2*2-1;

		vector<vector<double>> new_branch_flat_vals;
		new_branch_flat_vals.reserve(3);

		new_branch_flat_vals.push_back(vector<double>(2));
		new_branch_flat_vals[0][0] = rand()%2*2-1;
		if (new_branch_flat_vals[0][0] == 1.0) {
			second_val++;
		}
		new_branch_flat_vals[0][1] = rand()%2*2-1;

		new_branch_flat_vals.push_back(vector<double>(2));
		new_branch_flat_vals[1][0] = rand()%2*2-1;
		new_branch_flat_vals[1][1] = rand()%2*2-1;

		new_branch_flat_vals.push_back(vector<double>(3));
		new_branch_flat_vals[2][0] = rand()%2*2-1;
		if (new_branch_flat_vals[2][0] == 1.0) {
			second_val++;
		}
		new_branch_flat_vals[2][1] = rand()%2*2-1;
		new_branch_flat_vals[2][2] = rand()%2*2-1;

		vector<vector<double>> post_branch_flat_vals;
		post_branch_flat_vals.reserve(6);

		post_branch_flat_vals.push_back(vector<double>(2));
		post_branch_flat_vals[0][0] = rand()%2*2-1;
		post_branch_flat_vals[0][1] = rand()%2*2-1;

		post_branch_flat_vals.push_back(vector<double>(3));
		post_branch_flat_vals[1][0] = rand()%2*2-1;
		post_branch_flat_vals[1][1] = rand()%2*2-1;
		post_branch_flat_vals[1][2] = rand()%2*2-1;

		post_branch_flat_vals.push_back(vector<double>(2));
		post_branch_flat_vals[2][0] = rand()%2*2-1;
		second_score_modifier += post_branch_flat_vals[2][0];
		post_branch_flat_vals[2][1] = rand()%2*2-1;

		post_branch_flat_vals.push_back(vector<double>(2));
		post_branch_flat_vals[3][0] = rand()%2*2-1;
		if (post_branch_flat_vals[3][0] == 1.0) {
			shared_val++;
		}
		post_branch_flat_vals[3][1] = rand()%2*2-1;

		post_branch_flat_vals.push_back(vector<double>(2));
		post_branch_flat_vals[4][0] = rand()%2*2-1;
		shared_score_modifier += post_branch_flat_vals[4][0];
		post_branch_flat_vals[4][1] = rand()%2*2-1;

		post_branch_flat_vals.push_back(vector<double>(2));
		post_branch_flat_vals[5][0] = rand()%2*2-1;
		post_branch_flat_vals[5][1] = rand()%2*2-1;

		second_branch_fold_network->activate(pre_branch_flat_vals,
											 new_branch_flat_vals,
											 post_branch_flat_vals);

		double final_val;
		if (choice_val%2 == 0) {
			final_val = -5.0;
		} else {
			final_val = (shared_val+second_val)%2 + second_score_modifier + shared_score_modifier;
		}

		vector<double> errors;
		errors.push_back(final_val - second_branch_fold_network->output->acti_vals[0]);
		sum_error += abs(errors[0]);

		if (iter_index < 4000000) {
			second_branch_fold_network->backprop(errors, 0.01);
		} else {
			second_branch_fold_network->backprop(errors, 0.001);
		}
	}

	ofstream output_file;
	output_file.open("saves/second_branch_fold_network.txt");
	second_branch_fold_network->save(output_file);
	output_file.close();

	delete second_branch_fold_network;

	cout << "Done" << endl;
}
