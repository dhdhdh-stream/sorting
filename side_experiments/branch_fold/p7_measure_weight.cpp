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
	for (int i = 0; i < 11; i++) {
		ifstream input_file;
		input_file.open("saves/n_" + to_string(i) + "_10.txt");
		nodes.push_back(new Node(input_file));
		input_file.close();
	}

	double sum_score_diffs[11] = {};

	for (int iter_index = 0; iter_index < 10000; iter_index++) {
		vector<vector<double>> state_vals;
		double predicted_score = 0.0;

		int flat_input_counter = 0;

		int choice_val = 0;
		int xor_val = 0;
		double score_modifier = 0.0;

		vector<vector<double>> flat_vals;
		flat_vals.reserve(11);

		flat_vals.push_back(vector<double>(2));
		flat_vals[0][0] = rand()%2*2-1;
		flat_vals[0][1] = rand()%2*2-1;
		nodes[flat_input_counter]->activate(state_vals,
											flat_vals[0],
											predicted_score);
		if (nodes[flat_input_counter]->score_network != NULL) {
			sum_score_diffs[flat_input_counter] += abs(nodes[flat_input_counter]->
				score_network->output->acti_vals[0]);
		}
		flat_input_counter++;

		flat_vals.push_back(vector<double>(2));
		flat_vals[1][0] = rand()%2*2-1;
		if (flat_vals[1][0] == 1.0) {
			xor_val++;
		}
		flat_vals[1][1] = rand()%2*2-1;
		nodes[flat_input_counter]->activate(state_vals,
											flat_vals[1],
											predicted_score);
		if (nodes[flat_input_counter]->score_network != NULL) {
			sum_score_diffs[flat_input_counter] += abs(nodes[flat_input_counter]->
				score_network->output->acti_vals[0]);
		}
		flat_input_counter++;

		flat_vals.push_back(vector<double>(2));
		flat_vals[2][0] = rand()%2*2-1;
		score_modifier += flat_vals[2][0];
		flat_vals[2][1] = rand()%2*2-1;
		nodes[flat_input_counter]->activate(state_vals,
											flat_vals[2],
											predicted_score);
		if (nodes[flat_input_counter]->score_network != NULL) {
			sum_score_diffs[flat_input_counter] += abs(nodes[flat_input_counter]->
				score_network->output->acti_vals[0]);
		}
		flat_input_counter++;

		flat_vals.push_back(vector<double>(2));
		flat_vals[3][0] = rand()%2*2-1;
		if (flat_vals[3][0] == 1.0) {
			choice_val++;
		}
		flat_vals[3][1] = rand()%2*2-1;
		nodes[flat_input_counter]->activate(state_vals,
											flat_vals[3],
											predicted_score);
		if (nodes[flat_input_counter]->score_network != NULL) {
			sum_score_diffs[flat_input_counter] += abs(nodes[flat_input_counter]->
				score_network->output->acti_vals[0]);
		}
		flat_input_counter++;

		flat_vals.push_back(vector<double>(3));
		flat_vals[4][0] = rand()%2*2-1;
		if (flat_vals[4][0] == 1.0) {
			xor_val++;
		}
		flat_vals[4][1] = rand()%2*2-1;
		flat_vals[4][2] = rand()%2*2-1;
		nodes[flat_input_counter]->activate(state_vals,
											flat_vals[4],
											predicted_score);
		if (nodes[flat_input_counter]->score_network != NULL) {
			sum_score_diffs[flat_input_counter] += abs(nodes[flat_input_counter]->
				score_network->output->acti_vals[0]);
		}
		flat_input_counter++;

		flat_vals.push_back(vector<double>(2));
		flat_vals[5][0] = rand()%2*2-1;
		if (flat_vals[5][0] == 1.0) {
			choice_val++;
		}
		flat_vals[5][1] = rand()%2*2-1;
		nodes[flat_input_counter]->activate(state_vals,
											flat_vals[5],
											predicted_score);
		if (nodes[flat_input_counter]->score_network != NULL) {
			sum_score_diffs[flat_input_counter] += abs(nodes[flat_input_counter]->
				score_network->output->acti_vals[0]);
		}
		flat_input_counter++;

		flat_vals.push_back(vector<double>(3));
		flat_vals[6][0] = rand()%2*2-1;
		if (flat_vals[6][0] == 1.0) {
			xor_val++;
		}
		flat_vals[6][1] = rand()%2*2-1;
		flat_vals[6][2] = rand()%2*2-1;
		nodes[flat_input_counter]->activate(state_vals,
											flat_vals[6],
											predicted_score);
		if (nodes[flat_input_counter]->score_network != NULL) {
			sum_score_diffs[flat_input_counter] += abs(nodes[flat_input_counter]->
				score_network->output->acti_vals[0]);
		}
		flat_input_counter++;

		flat_vals.push_back(vector<double>(2));
		flat_vals[7][0] = rand()%2*2-1;
		flat_vals[7][1] = rand()%2*2-1;
		nodes[flat_input_counter]->activate(state_vals,
											flat_vals[7],
											predicted_score);
		if (nodes[flat_input_counter]->score_network != NULL) {
			sum_score_diffs[flat_input_counter] += abs(nodes[flat_input_counter]->
				score_network->output->acti_vals[0]);
		}
		flat_input_counter++;

		flat_vals.push_back(vector<double>(3));
		flat_vals[8][0] = rand()%2*2-1;
		score_modifier += flat_vals[8][0];
		flat_vals[8][1] = rand()%2*2-1;
		nodes[flat_input_counter]->activate(state_vals,
											flat_vals[8],
											predicted_score);
		if (nodes[flat_input_counter]->score_network != NULL) {
			sum_score_diffs[flat_input_counter] += abs(nodes[flat_input_counter]->
				score_network->output->acti_vals[0]);
		}
		flat_input_counter++;

		flat_vals.push_back(vector<double>(2));
		flat_vals[9][0] = rand()%2*2-1;
		if (flat_vals[9][0] == 1.0) {
			xor_val++;
		}
		flat_vals[9][1] = rand()%2*2-1;
		nodes[flat_input_counter]->activate(state_vals,
											flat_vals[9],
											predicted_score);
		if (nodes[flat_input_counter]->score_network != NULL) {
			sum_score_diffs[flat_input_counter] += abs(nodes[flat_input_counter]->
				score_network->output->acti_vals[0]);
		}
		flat_input_counter++;

		flat_vals.push_back(vector<double>(2));
		flat_vals[10][0] = rand()%2*2-1;
		flat_vals[10][1] = rand()%2*2-1;
		nodes[flat_input_counter]->activate(state_vals,
											flat_vals[10],
											predicted_score);
		if (nodes[flat_input_counter]->score_network != NULL) {
			sum_score_diffs[flat_input_counter] += abs(nodes[flat_input_counter]->
				score_network->output->acti_vals[0]);
		}
		flat_input_counter++;
	}

	for (int i = 0; i < 11; i++) {
		cout << i << ": " << sum_score_diffs[i]/10000 << endl;
	}

	for (int n_index = 0; n_index < (int)nodes.size(); n_index++) {
		delete nodes[n_index];
	}

	cout << "Done" << endl;
}
