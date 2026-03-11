#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "globals.h"
#include "helpers.h"
#include "minesweeper.h"
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

	ProblemType* problem_type = new TypeMinesweeper();

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

	for (int i_index = 0; i_index < 10000; i_index++) {
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

		iter_update_vals_helper(solution_wrapper->scope_histories[0],
								target_val);

		solution_wrapper->end();

		delete problem;
	}

	update_vals(solution_wrapper);

	for (int i_index = 0; i_index < 1000000; i_index++) {
		if (i_index % 100000 == 0) {
			cout << i_index << endl;
		}

		Problem* problem = problem_type->get_problem();

		solution_wrapper->init();

		while (true) {
			vector<double> obs = problem->get_observations();

			pair<bool,int> next = solution_wrapper->damage_step(obs);
			if (next.first) {
				break;
			} else {
				problem->perform_action(next.second);
			}
		}

		double target_val = problem->score_result();
		target_val -= 0.0001 * solution_wrapper->num_actions;

		iter_update_damage_helper(solution_wrapper->scope_histories[0],
								  target_val);

		solution_wrapper->end();

		delete problem;
	}

	update_damage(solution_wrapper);

	solution_wrapper->save("saves/", filename);

	delete problem_type;
	delete solution_wrapper;

	cout << "Done" << endl;
}
