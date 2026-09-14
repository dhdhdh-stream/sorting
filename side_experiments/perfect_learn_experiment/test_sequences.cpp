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

	geometric_distribution<int> geo_distribution(0.3);
	uniform_int_distribution<int> action_distribution(0, problem_type->num_possible_actions()-1);
	while (true) {
		int num_steps = geo_distribution(generator);

		vector<int> actions;
		for (int s_index = 0; s_index < num_steps; s_index++) {
			int action = action_distribution(generator);
			actions.push_back(action);
		}

		cout << "actions:";
		for (int a_index = 0; a_index < (int)actions.size(); a_index++) {
			cout << " " << actions[a_index];
		}
		cout << endl;

		focus(problem_type,
			  actions,
			  solution);

		compare(problem_type,
				actions);

		cout << endl;
	}

	delete problem_type;
	delete solution;

	cout << "Done" << endl;
}
