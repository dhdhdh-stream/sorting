#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "globals.h"
#include "solution.h"
#include "solution_set.h"

using namespace std;

int seed;

default_random_engine generator;

ProblemType* problem_type;
SolutionSet* solution_set;

#if defined(MDEBUG) && MDEBUG
int run_index = 0;
#endif /* MDEBUG */

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	solution_set = new SolutionSet();
	solution_set->init();

	solution_set->save("", "main");

	Solution* solution = solution_set->solutions[solution_set->curr_solution_index];

	ofstream display_file;
	display_file.open("../display.txt");
	solution->save_for_display(display_file);
	display_file.close();

	delete solution_set;

	cout << "Done" << endl;
}
