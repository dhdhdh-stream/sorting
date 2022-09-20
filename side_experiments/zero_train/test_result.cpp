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

	ifstream input_file;
	input_file.open("saves/f_10.txt");
	FoldNetwork* fold_network = new FoldNetwork(input_file);
	input_file.close();

	vector<Node*> nodes;
	for (int i = 0; i < 11; i++) {
		ifstream input_file;
		input_file.open("saves/n_" + to_string(i) + ".txt");
		nodes.push_back(new Node(i, input_file));
		input_file.close();
	}

	vector<vector<double>> state_vals;
	vector<bool> scopes_on;

	double flat_inputs[11];
	int flat_input_counter = 0;
	bool activated[11];
	for (int i = 0; i < 11; i++) {
		activated[i] = true;
	}

	// first block
	int first_block_sum = 0;
	for (int i = 0; i < 2; i++) {
		if (rand()%2 == 0) {
			flat_inputs[flat_input_counter] = 1.0;
			first_block_sum++;
		} else {
			flat_inputs[flat_input_counter] = 0.0;
		}
		nodes[flat_input_counter]->activate(state_vals,
											scopes_on,
											flat_inputs[flat_input_counter]);
		cout << "input: " << flat_inputs[flat_input_counter] << endl;
		for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
			cout << state_vals[sc_index][0] << endl;
		}
		flat_input_counter++;
	}

	// dud
	flat_inputs[flat_input_counter] = rand()%2;
	nodes[flat_input_counter]->activate(state_vals,
										scopes_on,
										flat_inputs[flat_input_counter]);
	cout << "input: " << flat_inputs[flat_input_counter] << endl;
	for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
		cout << state_vals[sc_index][0] << endl;
	}
	flat_input_counter++;

	for (int i = 0; i < 2; i++) {
		if (rand()%2 == 0) {
			flat_inputs[flat_input_counter] = 1.0;
			first_block_sum++;
		} else {
			flat_inputs[flat_input_counter] = 0.0;
		}
		nodes[flat_input_counter]->activate(state_vals,
											scopes_on,
											flat_inputs[flat_input_counter]);
		cout << "input: " << flat_inputs[flat_input_counter] << endl;
		for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
			cout << state_vals[sc_index][0] << endl;
		}
		flat_input_counter++;
	}

	// dud
	flat_inputs[flat_input_counter] = rand()%2;
	nodes[flat_input_counter]->activate(state_vals,
										scopes_on,
										flat_inputs[flat_input_counter]);
	cout << "input: " << flat_inputs[flat_input_counter] << endl;
	for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
		cout << state_vals[sc_index][0] << endl;
	}
	flat_input_counter++;

	// second block
	int second_block_sum = 0;
	for (int i = 0; i < 4; i++) {
		if (rand()%2 == 0) {
			flat_inputs[flat_input_counter] = 1.0;
			second_block_sum++;
		} else {
			flat_inputs[flat_input_counter] = 0.0;
		}
		nodes[flat_input_counter]->activate(state_vals,
											scopes_on,
											flat_inputs[flat_input_counter]);
		cout << "input: " << flat_inputs[flat_input_counter] << endl;
		for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
			cout << state_vals[sc_index][0] << endl;
		}
		flat_input_counter++;
	}

	// dud
	flat_inputs[flat_input_counter] = rand()%2;
	nodes[flat_input_counter]->activate(state_vals,
										scopes_on,
										flat_inputs[flat_input_counter]);
	cout << "input: " << flat_inputs[flat_input_counter] << endl;
	for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
		cout << state_vals[sc_index][0] << endl;
	}
	flat_input_counter++;

	int total_sum = 0;
	bool first_block_is_even = (first_block_sum%2 == 0);
	if (first_block_is_even) {
		total_sum += 7;
	}
	bool second_block_is_even = (second_block_sum%2 == 0);
	if (second_block_is_even) {
		total_sum += 5;
	}

	vector<double> obs;
	obs.push_back(rand()%2);

	fold_network->activate(flat_inputs,
						   activated,
						   obs,
						   state_vals);

	cout << "total_sum: " << total_sum << endl;
	cout << "fold_network->output->acti_vals[0]: " << fold_network->output->acti_vals[0] << endl;

	for (int n_index = 0; n_index < (int)nodes.size(); n_index++) {
		delete nodes[n_index];
	}
	delete fold_network;

	cout << "Done" << endl;
}
