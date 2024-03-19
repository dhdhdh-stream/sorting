// TODO: maybe learn 10 different for 20 changes each
// - then using those, learn 10 more different for 20 changes each
//   - but when saving, save 10 new along with 10 old
// - then with 20, learn 10 more different for 20 changes each
// - etc.

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"

using namespace std;

default_random_engine generator;

Problem* problem_type;
Solution* solution;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	Solution* new_solution = new Solution();
	new_solution->init();

	{
		solution = new Solution();
		solution->load("", "run_1");

		for (map<int, Scope*>::iterator it = solution->scopes.begin();
				it != solution->scopes.end(); it++) {
			it->second->id = new_solution->scope_counter;
			new_solution->scope_counter++;
			new_solution->scopes[it->second->id] = it->second;
		}
		solution->scopes.clear();

		if (solution->throw_counter > new_solution->throw_counter) {
			new_solution->throw_counter = solution->throw_counter;
		}

		if (solution->max_depth > new_solution->max_depth) {
			new_solution->max_depth = solution->max_depth;
		}

		if (solution->max_num_actions > new_solution->max_num_actions) {
			new_solution->max_num_actions = solution->max_num_actions;
		}

		delete solution;
	}

	{
		solution = new Solution();
		solution->load("", "run_2");

		for (map<int, Scope*>::iterator it = solution->scopes.begin();
				it != solution->scopes.end(); it++) {
			it->second->id = new_solution->scope_counter;
			new_solution->scope_counter++;
			new_solution->scopes[it->second->id] = it->second;
		}
		solution->scopes.clear();

		if (solution->throw_counter > new_solution->throw_counter) {
			new_solution->throw_counter = solution->throw_counter;
		}

		if (solution->max_depth > new_solution->max_depth) {
			new_solution->max_depth = solution->max_depth;
		}

		if (solution->max_num_actions > new_solution->max_num_actions) {
			new_solution->max_num_actions = solution->max_num_actions;
		}

		delete solution;
	}

	{
		solution = new Solution();
		solution->load("", "run_3");

		for (map<int, Scope*>::iterator it = solution->scopes.begin();
				it != solution->scopes.end(); it++) {
			it->second->id = new_solution->scope_counter;
			new_solution->scope_counter++;
			new_solution->scopes[it->second->id] = it->second;
		}
		solution->scopes.clear();

		if (solution->throw_counter > new_solution->throw_counter) {
			new_solution->throw_counter = solution->throw_counter;
		}

		if (solution->max_depth > new_solution->max_depth) {
			new_solution->max_depth = solution->max_depth;
		}

		if (solution->max_num_actions > new_solution->max_num_actions) {
			new_solution->max_num_actions = solution->max_num_actions;
		}

		delete solution;
	}

	new_solution->remap();

	new_solution->save("", "main");

	ofstream display_file;
	display_file.open("../display.txt");
	new_solution->save_for_display(display_file);
	display_file.close();

	delete new_solution;

	cout << "Done" << endl;
}
