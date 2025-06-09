// TODO: add back repeat
// - new scope, but follow immediately with decision making

// TODO: for CommitExperiment, can separate commit from information gather?

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "abstract_experiment.h"
#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "minesweeper.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "utilities.h"

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
		solution_wrapper = new SolutionWrapper(
			problem_type->num_obs(),
			problem_type->num_possible_actions(),
			"saves/",
			filename);
	} else {
		filename = "main.txt";
		solution_wrapper = new SolutionWrapper(
			problem_type->num_obs(),
			problem_type->num_possible_actions());
		solution_wrapper->save("saves/", filename);
	}

	solution_wrapper->save_for_display("../", "display.txt");

	while (solution_wrapper->solution->timestamp < EXPLORE_ITERS) {
		int starting_timestamp = solution_wrapper->solution->timestamp;

		Problem* problem = problem_type->get_problem();
		#if defined(MDEBUG) && MDEBUG
		solution_wrapper->problem = problem;
		#endif /* MDEBUG */

		solution_wrapper->experiment_init();

		vector<double> obs = problem->get_observations();
		while (true) {
			pair<bool,int> next = solution_wrapper->experiment_step(obs);
			if (next.first) {
				break;
			} else {
				problem->perform_action(next.second);
			}
		}

		double target_val = problem->score_result();

		solution_wrapper->experiment_end(target_val);

		delete problem;

		if (solution_wrapper->solution->timestamp > starting_timestamp) {
			#if defined(MDEBUG) && MDEBUG
			while (solution_wrapper->solution->verify_problems.size() > 0) {
				Problem* problem = solution_wrapper->solution->verify_problems[0];
				solution_wrapper->problem = problem;

				solution_wrapper->verify_init();

				vector<double> obs = problem->get_observations();
				while (true) {
					pair<bool,int> next = solution_wrapper->verify_step(obs);
					if (next.first) {
						break;
					} else {
						problem->perform_action(next.second);
					}
				}

				solution_wrapper->verify_end();

				delete solution_wrapper->solution->verify_problems[0];
				solution_wrapper->solution->verify_problems.erase(solution_wrapper->solution->verify_problems.begin());
			}
			solution_wrapper->solution->clear_verify();
			#endif /* MDEBUG */

			double sum_score = 0.0;
			for (int iter_index = 0; iter_index < MEASURE_ITERS; iter_index++) {
				Problem* problem = problem_type->get_problem();

				solution_wrapper->init();

				vector<double> obs = problem->get_observations();
				while (true) {
					pair<bool,int> next = solution_wrapper->step(obs);
					if (next.first) {
						break;
					} else {
						problem->perform_action(next.second);
					}
				}

				double target_val = problem->score_result();
				sum_score += target_val;

				solution_wrapper->end();

				delete problem;
			}

			cout << "curr_score: " << sum_score / MEASURE_ITERS << endl;

			solution_wrapper->save("saves/", filename);

			solution_wrapper->save_for_display("../", "display.txt");

			#if defined(MDEBUG) && MDEBUG
			delete solution_wrapper;
			solution_wrapper = new SolutionWrapper(
				problem_type->num_obs(),
				problem_type->num_possible_actions(),
				"saves/",
				filename);
			#endif /* MDEBUG */
		}
	}

	solution_wrapper->solution->clean_scopes();
	solution_wrapper->save("saves/", filename);

	delete problem_type;
	delete solution_wrapper;

	cout << "Done" << endl;
}
