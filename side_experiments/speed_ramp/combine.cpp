#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "abstract_experiment.h"
#include "globals.h"
#include "helpers.h"
#include "scope.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

int seed;

default_random_engine generator;

const int BRANCH_FACTOR = 4;

int main(int argc, char* argv[]) {
	if (argc != 1 + BRANCH_FACTOR + 1) {
		cout << "Usage: ./combine [target] [other ... children] [output]" << endl;
		exit(1);
	}

	string target_file = argv[1];
	vector<string> other_files(BRANCH_FACTOR-1);
	for (int c_index = 0; c_index < BRANCH_FACTOR-1; c_index++) {
		other_files[c_index] = argv[2 + c_index];
	}
	string output_file = argv[1 + BRANCH_FACTOR];

	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	SolutionWrapper* solution_wrapper = new SolutionWrapper(
		"saves/", target_file);

	for (int other_index = 0; other_index < BRANCH_FACTOR-1; other_index++) {
		solution_wrapper->combine("saves/", other_files[other_index]);
	}

	solution_wrapper->save("saves/", output_file);

	delete solution_wrapper;

	cout << "Done" << endl;
}
