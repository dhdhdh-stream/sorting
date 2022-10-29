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

#include "fold_network.h"
#include "network.h"
#include "node.h"
#include "scope.h"
#include "test_node.h"

using namespace std;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	vector<Node*> nodes;
	for (int i = 0; i < 15; i++) {
		ifstream input_file;
		input_file.open("saves/n_" + to_string(i) + "_14.txt");
		nodes.push_back(new Node(input_file));
		input_file.close();
	}

	Scope* scope = construct_scope(nodes);

	vector<int> explore_path;
	explore_path.push_back(1);
	int explore_start_inclusive = 3;
	int explore_end_non_inclusive = 4;

	vector<int> pre_branch_flat_sizes;
	pre_branch_flat_sizes.push_back(2);
	pre_branch_flat_sizes.push_back(2);

	{
		int choice_val = 0;
		int shared_val = 0;
		int second_val = 0;
		double second_score_modifier = 0.0;
		double shared_score_modifier = 0.0;

		vector<vector<double>> flat_vals;
		flat_vals.reserve(17);

		flat_vals.push_back(vector<double>(2));
		flat_vals[0][0] = rand()%2*2-1;
		flat_vals[0][1] = rand()%2*2-1;

		flat_vals.push_back(vector<double>(2));
		flat_vals[1][0] = rand()%2*2-1;
		if (flat_vals[1][0] == 1.0) {
			shared_val++;
		}
		flat_vals[1][1] = rand()%2*2-1;

		flat_vals.push_back(vector<double>(2));
		flat_vals[2][0] = rand()%2*2-1;
		flat_vals[2][1] = rand()%2*2-1;

		flat_vals.push_back(vector<double>(2));
		flat_vals[3][0] = rand()%2*2-1;
		second_score_modifier += flat_vals[3][0];
		flat_vals[3][1] = rand()%2*2-1;

		flat_vals.push_back(vector<double>(2));
		flat_vals[4][0] = rand()%2*2-1;
		if (flat_vals[4][0] == 1.0) {
			choice_val++;
		}
		flat_vals[4][1] = rand()%2*2-1;

		flat_vals.push_back(vector<double>(3));
		flat_vals[5][0] = rand()%2*2-1;
		flat_vals[5][1] = rand()%2*2-1;
		if (flat_vals[5][1] == 1.0) {
			second_val++;
		}
		flat_vals[5][2] = rand()%2*2-1;

		flat_vals.push_back(vector<double>(2));
		flat_vals[6][0] = rand()%2*2-1;
		if (flat_vals[6][0] == 1.0) {
			choice_val++;
		}
		flat_vals[6][1] = rand()%2*2-1;

		flat_vals.push_back(vector<double>(2));
		flat_vals[7][0] = rand()%2*2-1;
		flat_vals[7][1] = rand()%2*2-1;

		flat_vals.push_back(vector<double>(2));
		flat_vals[8][0] = rand()%2*2-1;
		if (flat_vals[8][0] == 1.0) {
			second_val++;
		}
		flat_vals[8][1] = rand()%2*2-1;

		flat_vals.push_back(vector<double>(2));
		flat_vals[9][0] = rand()%2*2-1;
		flat_vals[9][1] = rand()%2*2-1;

		flat_vals.push_back(vector<double>(3));
		flat_vals[10][0] = rand()%2*2-1;
		if (flat_vals[10][0] == 1.0) {
			second_val++;
		}
		flat_vals[10][1] = rand()%2*2-1;
		flat_vals[10][2] = rand()%2*2-1;

		flat_vals.push_back(vector<double>(2));
		flat_vals[11][0] = rand()%2*2-1;
		flat_vals[11][1] = rand()%2*2-1;

		flat_vals.push_back(vector<double>(3));
		flat_vals[12][0] = rand()%2*2-1;
		flat_vals[12][1] = rand()%2*2-1;
		flat_vals[12][2] = rand()%2*2-1;

		flat_vals.push_back(vector<double>(2));
		flat_vals[13][0] = rand()%2*2-1;
		second_score_modifier += flat_vals[13][0];
		flat_vals[13][1] = rand()%2*2-1;

		flat_vals.push_back(vector<double>(2));
		flat_vals[14][0] = rand()%2*2-1;
		if (flat_vals[14][0] == 1.0) {
			shared_val++;
		}
		flat_vals[14][1] = rand()%2*2-1;

		flat_vals.push_back(vector<double>(2));
		flat_vals[15][0] = rand()%2*2-1;
		shared_score_modifier += flat_vals[15][0];
		flat_vals[15][1] = rand()%2*2-1;

		flat_vals.push_back(vector<double>(2));
		flat_vals[16][0] = rand()%2*2-1;
		flat_vals[16][1] = rand()%2*2-1;

		vector<double> inputs;	// empty
		double predicted_score = 0.0;
		bool before_new = true;
		vector<vector<double>> pre_flat_vals;
		vector<vector<double>> new_flat_vals;
		vector<vector<double>> post_flat_vals;
		scope->explore_activate_flat(flat_vals,
									 inputs,
									 predicted_score,
									 explore_path,
									 explore_start_inclusive,
									 explore_end_non_inclusive,
									 before_new,
									 pre_flat_vals,
									 new_flat_vals,
									 post_flat_vals);

		for (int f_index = 0; f_index < (int)pre_flat_vals.size(); f_index++) {
			for (int o_index = 0; o_index < (int)pre_flat_vals[f_index].size(); o_index++) {
				cout << "pre " << f_index << " " << o_index << ": " << pre_flat_vals[f_index][o_index] << endl;
			}
		}

		for (int f_index = 0; f_index < (int)new_flat_vals.size(); f_index++) {
			for (int o_index = 0; o_index < (int)new_flat_vals[f_index].size(); o_index++) {
				cout << "new " << f_index << " " << o_index << ": " << new_flat_vals[f_index][o_index] << endl;
			}
		}

		for (int f_index = 0; f_index < (int)post_flat_vals.size(); f_index++) {
			for (int o_index = 0; o_index < (int)post_flat_vals[f_index].size(); o_index++) {
				cout << "post " << f_index << " " << o_index << ": " << post_flat_vals[f_index][o_index] << endl;
			}
		}
	}

	for (int n_index = 0; n_index < (int)nodes.size(); n_index++) {
		delete nodes[n_index];
	}
	delete scope;

	cout << "Done" << endl;
}
