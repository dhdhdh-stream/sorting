#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "abstract_decision_tree_node.h"
#include "decision_tree.h"

using namespace std;

int seed;

default_random_engine generator;

const int NUM_INPUTS = 10;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	DecisionTree* decision_tree = new DecisionTree();

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

		auto starting_history_size = decision_tree->improvement_history.size();

		decision_tree->backprop(inputs,
								target_val);

		if (starting_history_size != decision_tree->improvement_history.size()) {
			ofstream output_file;
			output_file.open("saves/main.txt");

			decision_tree->save(output_file);

			output_file.close();
		}
	}

	// ifstream input_file;
	// input_file.open("saves/main.txt");

	// decision_tree->load(input_file);

	// input_file.close();

	// // temp
	// cout << "decision_tree->root->id: " << decision_tree->root->id << endl;
	// cout << "decision_tree->nodes.size(): " << decision_tree->nodes.size() << endl;

	// {
	// 	vector<double> inputs;
	// 	double target_val = 0.0;

	// 	int val_1 = xor_distribution(generator);
	// 	inputs.push_back(val_1);
	// 	int val_2 = xor_distribution(generator);
	// 	inputs.push_back(val_2);
	// 	if (val_1 == val_2) {
	// 		target_val += 10.0;
	// 	} else {
	// 		target_val -= 10.0;
	// 	}

	// 	for (int i_index = 2; i_index < NUM_INPUTS; i_index++) {
	// 		double val = input_distribution(generator);
	// 		inputs.push_back(val);
	// 		target_val += val;
	// 	}

	// 	double val = decision_tree->activate(inputs);

	// 	cout << "val_1: " << val_1 << endl;
	// 	cout << "val_2: " << val_2 << endl;
	// 	cout << "target_val: " << target_val << endl;
	// 	cout << "val: " << val << endl;
	// }

	cout << "Done" << endl;
}
