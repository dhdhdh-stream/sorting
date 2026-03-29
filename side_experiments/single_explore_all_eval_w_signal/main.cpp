// - don't constantly update signals
//   - because explore/post signal needs to block
//     - and existing/pre signal needs to match

// TODO: one issue is that wil be dominated by early explore
// - and by the time middle explore, early already finished eval
//   - so late never explores

// - 50% single crazy explore, 50% all safe explore with signals

// TODO: always have at least 1 explore per run
// - pick first each run
//   - even if correlation, hopefully caught by ramp

// - only way to have more than 1 crazy explore per run is good signal
//   - where there is still no exit
//     - simply accept damage

// - what if train crazy, eval clean?
//   - doesn't make much sense without signals though
//     - each explore has to report a score for this to be worth it

// - can also split between explore, train new, and eval

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
			"saves/",
			filename);
	} else {
		filename = "main.txt";
		solution_wrapper = new SolutionWrapper(problem_type);
	}

	#if defined(MDEBUG) && MDEBUG
	while (true) {
	#else
	while (!solution_wrapper->is_done()) {
	#endif /* MDEBUG */
		int starting_timestamp = solution_wrapper->solution->timestamp;

		while (true) {
			Problem* problem = problem_type->get_problem();
			solution_wrapper->problem = problem;

			solution_wrapper->experiment_init();

			while (true) {
				vector<double> obs = problem->get_observations();

				tuple<bool,bool,int> next = solution_wrapper->experiment_step(obs);
				if (get<0>(next)) {
					break;
				} else if (get<1>(next)) {
					uniform_int_distribution<int> action_distribution(0, problem_type->num_possible_actions()-1);
					int new_action = action_distribution(generator);

					solution_wrapper->set_action(new_action);

					problem->perform_action(new_action);
				} else {
					problem->perform_action(get<2>(next));
				}
			}

			double target_val = problem->score_result();
			target_val -= 0.0001 * solution_wrapper->num_actions;

			solution_wrapper->experiment_end(target_val);

			delete problem;

			int ending_timestamp = solution_wrapper->solution->timestamp;
			if (ending_timestamp != starting_timestamp) {
				break;
			}
		}

		solution_wrapper->save("saves/", filename);

		solution_wrapper->save_for_display("../", "display.txt");

		#if defined(MDEBUG) && MDEBUG
		if (rand()%4 == 0) {
			delete solution_wrapper;
			solution_wrapper = new SolutionWrapper(
				"saves/",
				filename);
		}
		#endif /* MDEBUG */
	}

	solution_wrapper->clean_scopes();
	solution_wrapper->save("saves/", filename);

	delete problem_type;
	delete solution_wrapper;

	cout << "Done" << endl;
}
