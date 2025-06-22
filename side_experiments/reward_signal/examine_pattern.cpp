#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "globals.h"
#include "pattern.h"
#include "scope.h"
#include "simple.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

int seed;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	ProblemType* problem_type = new TypeSimple();

	SolutionWrapper* solution_wrapper = new SolutionWrapper(
		problem_type->num_obs(),
		"saves/",
		"main.txt");

	Pattern* pattern = solution_wrapper->solution->scopes[0]->pattern;
	for (int i_index = 0; i_index < (int)pattern->inputs.size(); i_index++) {
		pattern->inputs[i_index].print();
	}

	delete problem_type;
	delete solution_wrapper;

	cout << "Done" << endl;
}
