#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "abstract_experiment.h"
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

int multi_index = 0;

int run_index;

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

	problem_type = new TypeMinesweeper();

	solution = new Solution();
	solution->load("saves/", target_file);

	int existing_num_scopes = (int)solution->scopes.size();

	for (int other_index = 0; other_index < BRANCH_FACTOR-1; other_index++) {
		Solution* other_solution = new Solution();
		other_solution->load("saves/", other_files[other_index]);

		for (int scope_index = 1; scope_index < (int)other_solution->scopes.size(); scope_index++) {
			solution->scopes.push_back(other_solution->scopes[scope_index]);

			for (int i_index = 0; i_index < existing_num_scopes; i_index++) {
				solution->scopes[i_index]->child_scopes.push_back(other_solution->scopes[scope_index]);
			}
		}

		other_solution->scopes.erase(other_solution->scopes.begin() + 1, other_solution->scopes.end());

		delete other_solution;
	}

	for (int scope_index = 1; scope_index < (int)solution->scopes.size(); scope_index++) {
		solution->scopes[scope_index]->id = scope_index;
	}

	solution->timestamp = 0;
	solution->best_true_score_timestamp = 0;

	solution->save("saves/", output_file);

	delete problem_type;
	delete solution;

	cout << "Done" << endl;
}
