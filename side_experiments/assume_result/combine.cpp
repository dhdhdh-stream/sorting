#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "globals.h"
#include "scope.h"
#include "solution.h"

using namespace std;

int seed;

default_random_engine generator;

ProblemType* problem_type;
Solution* solution;

int run_index;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	Solution* combined_solution = new Solution();
	combined_solution->init();

	for (int existing_index = 0; existing_index < 30; existing_index++) {
		Solution* existing_solution = new Solution();
		existing_solution->load("", "main_" + to_string(existing_index) + "00");

		for (int scope_index = 1; scope_index < (int)existing_solution->scopes.size(); scope_index++) {
			combined_solution->scopes.push_back(existing_solution->scopes[scope_index]);
		}

		existing_solution->scopes.erase(existing_solution->scopes.begin() + 1, existing_solution->scopes.end());

		delete existing_solution;
	}

	for (int scope_index = 1; scope_index < (int)combined_solution->scopes.size(); scope_index++) {
		combined_solution->scopes[scope_index]->id = scope_index;
	}

	combined_solution->save("", "main");

	delete combined_solution;

	cout << "Done" << endl;
}
