/**
 * 0 - 2: blank
 * 1 - 3: 1 which is index
 * 2 - 3: 2 which are 1st val
 * 3 - 2: 1 which is 1st val
 * 4 - 3: blank
 * 5 - 4: 1 which is 1st val
 * 6 - 1: blank
 * 7 - 3: 1 which is 2nd val
 * 8 - 2: 2 which is 2nd val
 * 9 - 3: blank
 * 10 - 2: 1 which is 2nd val
 * 11 - 2: blank
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
	for (int i = 0; i < 12; i++) {
		ifstream input_file;
		input_file.open("saves/n_" + to_string(i) + "_11.txt");
		nodes.push_back(new Node(input_file));
		input_file.close();
	}

	Scope* scope = construct_scope(nodes);

	vector<vector<double>> state_vals;
	double predicted_score = 0.0;

	int flat_input_counter = 0;

	vector<vector<double>> flat_vals;
	flat_vals.reserve(12);

	flat_vals.push_back(vector<double>(2));
	flat_vals[0][0] = rand()%2*2-1;
	flat_vals[0][1] = rand()%2*2-1;
	nodes[flat_input_counter]->activate(state_vals,
										flat_vals[0],
										predicted_score);
	cout << flat_input_counter << endl;
	cout << "predicted_score: " << predicted_score << endl;
	flat_input_counter++;

	flat_vals.push_back(vector<double>(3));
	int index = rand()%4;
	flat_vals[1][0] = index;
	flat_vals[1][1] = rand()%2*2-1;
	flat_vals[1][2] = rand()%2*2-1;
	int index_sum = -1;
	nodes[flat_input_counter]->activate(state_vals,
										flat_vals[1],
										predicted_score);
	cout << flat_input_counter << endl;
	cout << "predicted_score: " << predicted_score << endl;
	cout << "state:" << endl;
	for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
		for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
			cout << sc_index << " " << st_index << ": " << state_vals[sc_index][st_index] << endl;
		}
	}
	cout << endl;
	flat_input_counter++;

	int first_sum = 0;

	flat_vals.push_back(vector<double>(3));
	flat_vals[2][0] = rand()%2*2-1;
	if (flat_vals[2][0] == 1.0) {
		first_sum++;
		if (index == 0) {
			index_sum++;
		}
	}
	flat_vals[2][1] = rand()%2*2-1;
	if (flat_vals[2][1] == 1.0) {
		first_sum++;
		if (index == 1) {
			index_sum++;
		}
	}
	flat_vals[2][2] = rand()%2*2-1;
	nodes[flat_input_counter]->activate(state_vals,
										flat_vals[2],
										predicted_score);
	cout << flat_input_counter << endl;
	cout << "predicted_score: " << predicted_score << endl;
	cout << "state:" << endl;
	for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
		for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
			cout << sc_index << " " << st_index << ": " << state_vals[sc_index][st_index] << endl;
		}
	}
	cout << endl;
	flat_input_counter++;

	flat_vals.push_back(vector<double>(2));
	flat_vals[3][0] = rand()%2*2-1;
	if (flat_vals[3][0] == 1.0) {
		first_sum++;
		if (index == 2) {
			index_sum++;
		}
	}
	flat_vals[3][1] = rand()%2*2-1;
	nodes[flat_input_counter]->activate(state_vals,
										flat_vals[3],
										predicted_score);
	cout << flat_input_counter << endl;
	cout << "predicted_score: " << predicted_score << endl;
	cout << "state:" << endl;
	for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
		for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
			cout << sc_index << " " << st_index << ": " << state_vals[sc_index][st_index] << endl;
		}
	}
	cout << endl;
	flat_input_counter++;

	flat_vals.push_back(vector<double>(3));
	flat_vals[4][0] = rand()%2*2-1;
	flat_vals[4][1] = rand()%2*2-1;
	flat_vals[4][2] = rand()%2*2-1;
	nodes[flat_input_counter]->activate(state_vals,
										flat_vals[4],
										predicted_score);
	cout << flat_input_counter << endl;
	cout << "predicted_score: " << predicted_score << endl;
	cout << "state:" << endl;
	for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
		for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
			cout << sc_index << " " << st_index << ": " << state_vals[sc_index][st_index] << endl;
		}
	}
	cout << endl;
	flat_input_counter++;

	flat_vals.push_back(vector<double>(4));
	flat_vals[5][0] = rand()%2*2-1;
	if (flat_vals[5][0] == 1.0) {
		first_sum++;
		if (index == 3) {
			index_sum++;
		}
	}
	flat_vals[5][1] = rand()%2*2-1;
	flat_vals[5][2] = rand()%2*2-1;
	flat_vals[5][3] = rand()%2*2-1;
	nodes[flat_input_counter]->activate(state_vals,
										flat_vals[5],
										predicted_score);
	cout << flat_input_counter << endl;
	cout << "predicted_score: " << predicted_score << endl;
	cout << "state:" << endl;
	for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
		for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
			cout << sc_index << " " << st_index << ": " << state_vals[sc_index][st_index] << endl;
		}
	}
	cout << endl;
	flat_input_counter++;

	flat_vals.push_back(vector<double>(1));
	flat_vals[6][0] = rand()%2*2-1;
	nodes[flat_input_counter]->activate(state_vals,
										flat_vals[6],
										predicted_score);
	cout << flat_input_counter << endl;
	cout << "predicted_score: " << predicted_score << endl;
	cout << "state:" << endl;
	for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
		for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
			cout << sc_index << " " << st_index << ": " << state_vals[sc_index][st_index] << endl;
		}
	}
	cout << endl;
	flat_input_counter++;

	int second_sum = 0;

	flat_vals.push_back(vector<double>(3));
	flat_vals[7][0] = rand()%2*2-1;
	if (flat_vals[7][0] == 1.0) {
		second_sum++;
		if (index == 0) {
			index_sum++;
		}
	}
	flat_vals[7][1] = rand()%2*2-1;
	flat_vals[7][2] = rand()%2*2-1;
	nodes[flat_input_counter]->activate(state_vals,
										flat_vals[7],
										predicted_score);
	cout << flat_input_counter << endl;
	cout << "predicted_score: " << predicted_score << endl;
	cout << "state:" << endl;
	for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
		for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
			cout << sc_index << " " << st_index << ": " << state_vals[sc_index][st_index] << endl;
		}
	}
	cout << endl;
	flat_input_counter++;

	flat_vals.push_back(vector<double>(2));
	flat_vals[8][0] = rand()%2*2-1;
	if (flat_vals[8][0] == 1.0) {
		second_sum++;
		if (index == 1) {
			index_sum++;
		}
	}
	flat_vals[8][1] = rand()%2*2-1;
	if (flat_vals[8][1] == 1.0) {
		second_sum++;
		if (index == 2) {
			index_sum++;
		}
	}
	nodes[flat_input_counter]->activate(state_vals,
										flat_vals[8],
										predicted_score);
	cout << flat_input_counter << endl;
	cout << "predicted_score: " << predicted_score << endl;
	cout << "state:" << endl;
	for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
		for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
			cout << sc_index << " " << st_index << ": " << state_vals[sc_index][st_index] << endl;
		}
	}
	cout << endl;
	flat_input_counter++;

	flat_vals.push_back(vector<double>(3));
	flat_vals[9][0] = rand()%2*2-1;
	flat_vals[9][1] = rand()%2*2-1;
	flat_vals[9][2] = rand()%2*2-1;
	nodes[flat_input_counter]->activate(state_vals,
										flat_vals[9],
										predicted_score);
	cout << flat_input_counter << endl;
	cout << "predicted_score: " << predicted_score << endl;
	cout << "state:" << endl;
	for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
		for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
			cout << sc_index << " " << st_index << ": " << state_vals[sc_index][st_index] << endl;
		}
	}
	cout << endl;
	flat_input_counter++;

	flat_vals.push_back(vector<double>(2));
	flat_vals[10][0] = rand()%2*2-1;
	if (flat_vals[10][0] == 1.0) {
		second_sum++;
		if (index == 3) {
			index_sum++;
		}
	}
	flat_vals[10][1] = rand()%2*2-1;
	nodes[flat_input_counter]->activate(state_vals,
										flat_vals[10],
										predicted_score);
	cout << flat_input_counter << endl;
	cout << "predicted_score: " << predicted_score << endl;
	cout << "state:" << endl;
	for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
		for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
			cout << sc_index << " " << st_index << ": " << state_vals[sc_index][st_index] << endl;
		}
	}
	cout << endl;
	flat_input_counter++;

	flat_vals.push_back(vector<double>(2));
	flat_vals[11][0] = rand()%2*2-1;
	flat_vals[11][1] = rand()%2*2-1;
	nodes[flat_input_counter]->activate(state_vals,
										flat_vals[11],
										predicted_score);
	cout << flat_input_counter << endl;
	cout << "predicted_score: " << predicted_score << endl;
	cout << "state:" << endl;
	for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
		for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
			cout << sc_index << " " << st_index << ": " << state_vals[sc_index][st_index] << endl;
		}
	}
	cout << endl;
	flat_input_counter++;

	double final_val = first_sum%2 - second_sum%2 + index_sum;

	// empty
	vector<double> inputs;
	vector<double> outputs;
	double scope_predicted_score = 0.0;
	scope->activate(flat_vals,
					inputs,
					outputs,
					scope_predicted_score);

	cout << "final_val: " << final_val << endl;

	for (int n_index = 0; n_index < (int)nodes.size(); n_index++) {
		delete nodes[n_index];
	}
	delete scope;

	cout << "Done" << endl;
}
