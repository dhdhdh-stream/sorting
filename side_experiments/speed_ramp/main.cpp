// TODO: improve explore quality
// - world modeling, etc.
// - result will probably also help long problems
// - actions include complex actions, and exiting to certain parts of the solution

// TODO: try really cutting down on number of samples
// - really rely on rampup to make things safe

// - when combining, only try scopes that generalize

// TODO: do some calculations on number of inputs and chance for one to randomly correlate
// - vs number of samples
//   - I guess really bad because selecting most correlated from infinite obs
// - maybe 2 step it
//   - use correlation to choose what to pay attention to
//     - then regather samples
//   - how to handle XORs though?

// - remember that without every dependency, XOR cannot be learned
//   - (but with every dependency, can be learned incrementally)
//     - (begin by recognizing 1 pattern, then generalize to rest)

// - check if XOR, both dependencies can be randomly correlated to result
//   - I suspect can't

// TODO: could do a more gradual train:
// - while waffling between 2 options, have current be current decision making
//   - on explore, choose opposite
//     - then retrain
// - not really different from trying a fresh explore though?
//   - I guess corrects both ways at once instead of single way

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
#include "helpers.h"
// #include "instance_scores.h"
#include "minesweeper.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
// #include "simpler.h"
#include "solution.h"
#include "solution_wrapper.h"
#include "start_node.h"
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

	// ProblemType* problem_type = new TypeInstanceScores();
	// ProblemType* problem_type = new TypeSimpler();
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
		solution_wrapper = new SolutionWrapper();
	}

	int iter_index = 0;

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

			solution_wrapper->experiment_end(target_val);

			delete problem;

			iter_index++;
			if (iter_index%100000 == 0) {
				cout << "iter_index: " << iter_index << endl;
			}

			if (solution_wrapper->solution->timestamp != starting_timestamp) {
				break;
			}
		}

		solution_wrapper->save("saves/", filename);

		solution_wrapper->save_for_display("../", "display.txt");
	}

	solution_wrapper->clean_scopes();
	solution_wrapper->save("saves/", filename);

	delete problem_type;
	delete solution_wrapper;

	cout << "Done" << endl;
}
