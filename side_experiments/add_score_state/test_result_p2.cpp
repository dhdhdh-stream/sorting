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
	input_file.open("saves/p2_f_9.txt");
	FoldNetwork* fold_network = new FoldNetwork(input_file);
	input_file.close();

	vector<Node*> nodes;
	for (int i = 0; i < 10; i++) {
		ifstream input_file;
		input_file.open("saves/p2_n_" + to_string(i) + "_9.txt");
		nodes.push_back(new Node("p2_n_"+to_string(i), input_file));
		input_file.close();
	}

	vector<vector<double>> state_vals;
	vector<bool> scopes_on;

	double flat_inputs[10];
	int flat_input_counter = 0;
	bool activated[10];
	for (int i = 0; i < 10; i++) {
		activated[i] = true;
	}

	// first block
	int first_block_on_index = rand()%5;
	flat_inputs[flat_input_counter] = first_block_on_index;
	nodes[flat_input_counter]->activate(state_vals,
										scopes_on,
										flat_inputs[flat_input_counter]);
	cout << "input: " << flat_inputs[flat_input_counter] << endl;
	for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
		cout << state_vals[sc_index][0] << endl;
	}
	flat_input_counter++;

	int first_block_sum = 0;
	for (int i = 0; i < 4; i++) {
		int value = rand()%4;
		flat_inputs[flat_input_counter] = value;
		if (i < first_block_on_index) {
			first_block_sum += value;
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

	// second block
	int second_block_on_index = rand()%5;
	flat_inputs[flat_input_counter] = second_block_on_index;
	nodes[flat_input_counter]->activate(state_vals,
										scopes_on,
										flat_inputs[flat_input_counter]);
	cout << "input: " << flat_inputs[flat_input_counter] << endl;
	for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
		cout << state_vals[sc_index][0] << endl;
	}
	flat_input_counter++;

	int second_block_sum = 0;
	for (int i = 0; i < 4; i++) {
		int value = rand()%4;
		flat_inputs[flat_input_counter] = value;
		if (i < second_block_on_index) {
			second_block_sum += value;
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

	int final_value = first_block_sum - second_block_sum;

	vector<double> obs;
	obs.push_back(rand()%2);

	fold_network->activate(flat_inputs,
						   activated,
						   obs,
						   state_vals);

	cout << "final_value: " << final_value << endl;
	cout << "fold_network->output->acti_vals[0]: " << fold_network->output->acti_vals[0] << endl;

	for (int n_index = 0; n_index < (int)nodes.size(); n_index++) {
		delete nodes[n_index];
	}
	delete fold_network;

	cout << "Done" << endl;
}
