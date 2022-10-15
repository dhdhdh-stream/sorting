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
	cout << "scope->actions.size(): " << scope->actions.size() << endl;

	for (int n_index = 0; n_index < (int)nodes.size(); n_index++) {
		delete nodes[n_index];
	}

	cout << "Done" << endl;
}
