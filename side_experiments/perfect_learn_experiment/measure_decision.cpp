#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "action_network.h"
#include "constants.h"
#include "minesweeper.h"
#include "obs_network.h"
#include "score_network.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

int seed;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	ProblemType* problem_type = new TypeMinesweeper();

	string filename;
	Solution* solution;
	if (argc > 1) {
		filename = argv[1];
	} else {
		filename = "main.txt";
	}
	solution = new Solution(
		"saves/",
		filename);

	vector<int> actions_1;
	vector<int> actions_2{0, 0, 1, 5};

	predict_decision_helper(problem_type,
							actions_1,
							actions_2,
							solution);

	train_decision_helper(problem_type,
						  actions_1,
						  actions_2);

	delete problem_type;
	delete solution;

	cout << "Done" << endl;
}
