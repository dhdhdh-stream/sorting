#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "globals.h"
#include "helpers.h"
#include "signal_experiment.h"
#include "simpler.h"
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

	TypeSimpler* problem_type = new TypeSimpler();

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
		cout << "s_index: " << s_index << endl;

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

		problem->print();
		double signal = calc_signal(solution_wrapper->scope_histories[0],
									solution_wrapper);
		cout << "signal: " << signal << endl;

		solution_wrapper->end();

		delete problem;
	}

	/**
	 * - examine positive generalization
	 */
	uniform_int_distribution<int> positive_distribution(1, 10);
	for (int s_index = 0; s_index < (int)solution_wrapper->solutions.size(); s_index++) {
		cout << "s_index: " << s_index << endl;

		Simpler* problem = (Simpler*)problem_type->get_problem();
		problem->world[2] += positive_distribution(generator);

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

		problem->print();
		double signal = calc_signal(solution_wrapper->scope_histories[0],
									solution_wrapper);
		cout << "signal: " << signal << endl;

		solution_wrapper->end();

		delete problem;
	}

	delete problem_type;
	delete solution_wrapper;

	cout << "Done" << endl;
}
