#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "abstract_node.h"
#include "globals.h"
#include "helpers.h"
#include "instance_scores.h"
#include "scope.h"
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

	ProblemType* problem_type = new TypeInstanceScores();

	string filename;
	SolutionWrapper* solution_wrapper;
	if (argc > 1) {
		filename = argv[1];
	} else {
		filename = "main.txt";
	}
	solution_wrapper = new SolutionWrapper(
		"saves/",
		filename);

	{
		Problem* problem = problem_type->get_problem();

		solution_wrapper->init();

		while (true) {
			vector<double> obs = problem->get_observations();

			pair<bool,int> next = solution_wrapper->step(obs);
			if (next.first) {
				break;
			} else {
				problem->perform_action(next.second);
			}
		}

		double target_val = problem->score_result();
		cout << "target_val: " << target_val << endl;

		solution_wrapper->end();

		problem->print();

		cout << "solution_wrapper->num_actions: " << solution_wrapper->num_actions << endl;

		delete problem;
	}

	solution_wrapper->save_for_display("../", "display.txt");

	delete problem_type;
	delete solution_wrapper;

	cout << "Done" << endl;
}
