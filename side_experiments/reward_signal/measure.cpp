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

	double sum_vals = 0.0;
	int max_num_actions = 0;

	auto start_time = chrono::high_resolution_clock::now();
	for (int i_index = 0; i_index < 2000; i_index++) {
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
		target_val -= 0.0001 * solution_wrapper->num_actions;

		solution_wrapper->end();

		if (solution_wrapper->num_actions > max_num_actions) {
			max_num_actions = solution_wrapper->num_actions;
		}

		sum_vals += target_val;

		delete problem;
	}

	cout << "average score: " << sum_vals/2000 << endl;
	cout << "max_num_actions: " << max_num_actions << endl;

	auto curr_time = chrono::high_resolution_clock::now();
	auto time_diff = chrono::duration_cast<chrono::milliseconds>(curr_time - start_time);
	cout << "time_diff.count(): " << time_diff.count() << endl;

	solution_wrapper->save_for_display("../", "display.txt");

	delete problem_type;
	delete solution_wrapper;

	cout << "Done" << endl;
}
