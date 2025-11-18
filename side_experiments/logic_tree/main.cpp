// - if don't want separate signal actions
//   - can try explicitly including nodes as pre vs. post

// - BTW, for scopes, context can be useful to pass in as an input
//   - number of actions
//   - number of times scope has already appeared

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "logic_helpers.h"
#include "logic_tree.h"
#include "logic_wrapper.h"
#include "moving_signal.h"

using namespace std;

int seed;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	AbstractProblem* problem = new MovingSignal();

	string filename;
	LogicWrapper* logic_wrapper;
	if (argc > 1) {
		filename = argv[1];
		logic_wrapper = new LogicWrapper(
			"saves/",
			filename);
	} else {
		filename = "main.txt";
		logic_wrapper = new LogicWrapper(problem);
	}

	while (true) {
		auto starting_num_nodes = logic_wrapper->logic_tree->nodes.size();

		vector<double> obs;
		double target_val;
		problem->get_instance(obs,
							  target_val);

		logic_experiment_helper(logic_wrapper->logic_tree->root,
								obs,
								target_val,
								logic_wrapper);

		logic_wrapper->update(problem);

		if (logic_wrapper->logic_tree->nodes.size() != starting_num_nodes) {
			logic_wrapper->save("saves/", filename);
		}
	}

	delete problem;
	delete logic_wrapper;

	cout << "Done" << endl;
}
