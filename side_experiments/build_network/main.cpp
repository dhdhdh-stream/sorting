// - not good enough
//   - try training new while modifying old

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "build_network.h"

using namespace std;

int seed;

default_random_engine generator;

// const int NUM_INPUTS = 100;
const int NUM_INPUTS = 20;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	BuildNetwork* build_network = new BuildNetwork(NUM_INPUTS);

	uniform_int_distribution<int> xor_distribution(0, 1);
	uniform_int_distribution<int> input_distribution(-10, 10);
	for (int iter_index = 0; iter_index < 100000; iter_index++) {
		vector<double> inputs;
		double target_val = 0.0;

		int val_1 = xor_distribution(generator);
		inputs.push_back(val_1);
		int val_2 = xor_distribution(generator);
		inputs.push_back(val_2);
		if (val_1 == val_2) {
			target_val += 10.0;
		} else {
			target_val -= 10.0;
		}

		for (int i_index = 2; i_index < NUM_INPUTS; i_index++) {
			double val = input_distribution(generator);
			inputs.push_back(val);
			target_val += val;
		}

		auto starting_num_nodes = build_network->nodes.size();

		build_network->backprop(inputs,
								target_val);

		if (starting_num_nodes != build_network->nodes.size()) {
			ofstream output_file;
			output_file.open("saves/main.txt");

			build_network->save(output_file);

			output_file.close();
		}
	}

	cout << "Done" << endl;
}
