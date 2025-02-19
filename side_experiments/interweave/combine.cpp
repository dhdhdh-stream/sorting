#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "abstract_experiment.h"
#include "constants.h"
#include "globals.h"
#include "minesweeper.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

int seed;

default_random_engine generator;

ProblemType* problem_type;
Solution* solution;

int run_index = 0;

const int BRANCH_FACTOR = 4;

int main(int argc, char* argv[]) {
	if (argc != 1 + BRANCH_FACTOR + 1) {
		cout << "Usage: ./combine [child ... files] [output]" << endl;
		exit(1);
	}
	
	vector<string> child_files(BRANCH_FACTOR);
	for (int c_index = 0; c_index < BRANCH_FACTOR; c_index++) {
		child_files[c_index] = argv[1 + c_index];
	}
	string output_file = argv[1 + BRANCH_FACTOR];

	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	problem_type = new TypeMinesweeper();

	solution = new Solution();
	solution->init();

	for (int existing_index = 0; existing_index < BRANCH_FACTOR; existing_index++) {
		Solution* existing_solution = new Solution();
		existing_solution->load("saves/", child_files[existing_index]);

		for (int scope_index = 1; scope_index < (int)existing_solution->scopes.size(); scope_index++) {
			solution->scopes.push_back(existing_solution->scopes[scope_index]);
		}

		existing_solution->scopes.erase(existing_solution->scopes.begin() + 1, existing_solution->scopes.end());

		delete existing_solution;
	}

	for (int scope_index = 0; scope_index < (int)solution->scopes.size(); scope_index++) {
		/**
		 * - scopes[0] not added yet
		 */
		solution->scopes[scope_index]->id = 1 + scope_index;
	}

	for (int scope_index = 0; scope_index < (int)solution->scopes.size(); scope_index++) {
		solution->existing_scopes.push_back(solution->scopes[scope_index]);
	}

	solution->init();

	solution->save("saves/", output_file);

	delete problem_type;
	delete solution;

	cout << "Done" << endl;
}
