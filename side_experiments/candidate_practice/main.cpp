// - how to find candidates?
//   - look for actions and their impact first?
//     - thing is can't fixate on actions initially
//       - so can't distinguish if a pattern is or isn't actionable
//       - only way to know if an action is actionable is to chase
//   - perform random amounts of random actions and look for patterns?
//     - what's a pattern?
//       - just pick some number of obs
//       - filter to 5% and train on samples
//   - test how impactful noise is?
//     - obviously, enough noise is just like true, and will fail
//   - look for patterns of different sizes
//     - even 1 can be good
//       - i.e., where nothing else besides a single spot needs to matter

// - candidates then need to be refined and extended
// - what does it mean for a tunnel to be better?
// - don't worry about generalization
//   - can generalize in multiple directions anyways, so no simple answer
//     - instead, constantly retrain tunnel/look for new ones

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
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "tunnel_practice.h"
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

	ProblemType* problem_type = new TypeTunnelPractice();

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
		int starting_timestamp;
		if (solution_wrapper->curr_solution != NULL) {
			starting_timestamp = solution_wrapper->curr_solution->timestamp;
		} else {
			starting_timestamp = solution_wrapper->prev_solution->timestamp;
		}

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

			int ending_timestamp;
			if (solution_wrapper->curr_solution != NULL) {
				ending_timestamp = solution_wrapper->curr_solution->timestamp;
			} else {
				ending_timestamp = solution_wrapper->prev_solution->timestamp;
			}
			if (ending_timestamp != starting_timestamp) {
				/**
				 * - simply don't bother verifying on tunnel changes
				 */
				#if defined(MDEBUG) && MDEBUG
				while (solution_wrapper->curr_solution->verify_problems.size() > 0) {
					Problem* problem = solution_wrapper->curr_solution->verify_problems[0];
					solution_wrapper->problem = problem;

					solution_wrapper->verify_init();

					while (true) {
						vector<double> obs = problem->get_observations();

						pair<bool,int> next = solution_wrapper->verify_step(obs);
						if (next.first) {
							break;
						} else {
							problem->perform_action(next.second);
						}
					}

					solution_wrapper->verify_end();

					delete solution_wrapper->curr_solution->verify_problems[0];
					solution_wrapper->curr_solution->verify_problems.erase(solution_wrapper->curr_solution->verify_problems.begin());
				}
				solution_wrapper->curr_solution->clear_verify();
				#endif /* MDEBUG */

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
