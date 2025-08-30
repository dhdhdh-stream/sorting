#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "globals.h"
#include "scope.h"
#include "simpler.h"
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

	ProblemType* problem_type = new TypeSimpler();

	string filename;
	SolutionWrapper* solution_wrapper;
	if (argc > 1) {
		filename = argv[1];
	} else {
		filename = "main.txt";
	}
	solution_wrapper = new SolutionWrapper(
		problem_type->num_obs(),
		"saves/",
		filename);

	for (int s_index = 0; s_index < (int)solution_wrapper->solutions.size(); s_index++) {
		double sum_vals = 0.0;
		for (int i_index = 0; i_index < 2000; i_index++) {
			Problem* problem = problem_type->get_problem();

			solution_wrapper->init(s_index);

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
			target_val -= 0.0001 * solution_wrapper->num_actions;

			solution_wrapper->end();

			sum_vals += target_val;

			delete problem;
		}

		cout << "solution_wrapper->solutions[s_index]->curr_val_average: " << solution_wrapper->solutions[s_index]->curr_val_average << endl;
		cout << "average score: " << sum_vals/2000 << endl;
	}

	delete problem_type;
	delete solution_wrapper;

	cout << "Done" << endl;
}
