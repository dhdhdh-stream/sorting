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

	double average_score = 0.0;

	vector<Node*> nodes;
	for (int i = 0; i < 10; i++) {
		ifstream input_file;
		input_file.open("saves/r2_p2_n_" + to_string(i) + "_9.txt");
		nodes.push_back(new Node("r2_p2_n_"+to_string(i), input_file));
		input_file.close();
	}

	vector<Node*> zero_nodes;
	for (int i = 0; i < 10; i++) {
		ifstream input_file;
		// input_file.open("saves/r2_p2_n_" + to_string(i) + "_c_zt.txt");
		input_file.open("saves/r2_p2_n_" + to_string(i) + "_9.txt");
		// zero_nodes.push_back(new Node("r2_p2_n_"+to_string(i)+"_c", input_file));
		zero_nodes.push_back(new Node("r2_p2_n_"+to_string(i), input_file));
		input_file.close();
	}

	vector<vector<double>> state_vals;
	state_vals.push_back(vector<double>{average_score});
	vector<bool> scopes_on;
	scopes_on.push_back(true);

	vector<vector<double>> zero_state_vals = state_vals;
	vector<bool> zero_scopes_on = scopes_on;

	int flat_input_counter = 0;

	// first block
	int first_block_on_index = rand()%5;
	nodes[flat_input_counter]->activate(state_vals,
										scopes_on,
										first_block_on_index);
	zero_nodes[flat_input_counter]->activate(zero_state_vals,
											 zero_scopes_on,
											 first_block_on_index);
	cout << "input: " << first_block_on_index << endl;
	cout << "state_vals: " << endl;
	for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
		cout << state_vals[sc_index][0] << endl;
	}
	cout << "zero_state_vals: " << endl;
	for (int sc_index = 0; sc_index < (int)zero_state_vals.size(); sc_index++) {
		cout << zero_state_vals[sc_index][0] << endl;
	}
	flat_input_counter++;

	// for (int sc_index = 0; sc_index < (int)zero_state_vals.size()-1; sc_index++) {
	// 	for (int st_index = 0; st_index < (int)zero_state_vals[sc_index].size(); st_index++) {
	// 		zero_state_vals[sc_index][st_index] = 0.0;
	// 	}
	// 	zero_scopes_on[sc_index] = false;
	// }

	int first_block_sum = 0;
	for (int i = 0; i < 4; i++) {
		int value = rand()%4;
		if (i < first_block_on_index) {
			first_block_sum += value;
		}
		nodes[flat_input_counter]->activate(state_vals,
											scopes_on,
											value);
		zero_nodes[flat_input_counter]->activate(zero_state_vals,
												 zero_scopes_on,
												 value);
		cout << "input: " << value << endl;
		cout << "state_vals: " << endl;
		for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
			cout << state_vals[sc_index][0] << endl;
		}
		cout << "zero_state_vals: " << endl;
		for (int sc_index = 0; sc_index < (int)zero_state_vals.size(); sc_index++) {
			cout << zero_state_vals[sc_index][0] << endl;
		}
		flat_input_counter++;
	}

	// second block
	int second_block_on_index = rand()%5;
	nodes[flat_input_counter]->activate(state_vals,
										scopes_on,
										second_block_on_index);
	zero_nodes[flat_input_counter]->activate(zero_state_vals,
											 zero_scopes_on,
											 second_block_on_index);
	cout << "input: " << second_block_on_index << endl;
	cout << "state_vals: " << endl;
	for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
		cout << state_vals[sc_index][0] << endl;
	}
	cout << "zero_state_vals: " << endl;
	for (int sc_index = 0; sc_index < (int)zero_state_vals.size(); sc_index++) {
		cout << zero_state_vals[sc_index][0] << endl;
	}
	flat_input_counter++;

	for (int sc_index = 0; sc_index < (int)zero_state_vals.size()-1; sc_index++) {
		for (int st_index = 0; st_index < (int)zero_state_vals[sc_index].size(); st_index++) {
			zero_state_vals[sc_index][st_index] = 0.0;
		}
		zero_scopes_on[sc_index] = false;
	}

	int second_block_sum = 0;
	for (int i = 0; i < 4; i++) {
		int value = rand()%4;
		if (i < second_block_on_index) {
			second_block_sum += value;
		}
		nodes[flat_input_counter]->activate(state_vals,
											scopes_on,
											value);
		zero_nodes[flat_input_counter]->activate(zero_state_vals,
												 zero_scopes_on,
												 value);
		cout << "input: " << value << endl;
		cout << "state_vals: " << endl;
		for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
			cout << state_vals[sc_index][0] << endl;
		}
		cout << "zero_state_vals: " << endl;
		for (int sc_index = 0; sc_index < (int)zero_state_vals.size(); sc_index++) {
			cout << zero_state_vals[sc_index][0] << endl;
		}
		flat_input_counter++;
	}

	for (int n_index = 0; n_index < (int)nodes.size(); n_index++) {
		delete nodes[n_index];
	}
	for (int n_index = 0; n_index < (int)zero_nodes.size(); n_index++) {
		delete zero_nodes[n_index];
	}

	cout << "Done" << endl;
}
