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
	} else {
		filename = "main.txt";
	}
	logic_wrapper = new LogicWrapper(
		"saves/",
		filename);

	vector<double> obs;
	double target_val;
	problem->get_instance(obs,
						  target_val);

	double eval = logic_eval_helper(logic_wrapper->logic_tree->root,
									obs);

	for (int x_index = 0; x_index < 5; x_index++) {
		for (int y_index = 0; y_index < 5; y_index++) {
			cout << obs[x_index * 5 + y_index] << " ";
		}
		cout << endl;
	}
	cout << "target_val: " << target_val << endl;
	cout << "eval: " << eval << endl;

	delete problem;
	delete logic_wrapper;

	cout << "Done" << endl;
}
