// TODO: try adding curr score to obs

// TODO: for rewind, instead of comparing last, average last 3
// - and don't rewind multiple

// - currently, tunnels bad on average
//   - so any selected tunnels are more likely noise
//     - maybe have tunnels proven to be reasonable first before starting to chase?

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
// #include "tunnel_practice.h"
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

	// ProblemType* problem_type = new TypeTunnelPractice();
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
				#if defined(MDEBUG) && MDEBUG
				while (solution_wrapper->solution->verify_problems.size() > 0) {
					Problem* problem = solution_wrapper->solution->verify_problems[0];
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

					delete solution_wrapper->solution->verify_problems[0];
					solution_wrapper->solution->verify_problems.erase(solution_wrapper->solution->verify_problems.begin());
				}
				solution_wrapper->solution->clear_verify();
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
