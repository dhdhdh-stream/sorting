// - maybe key is to look for safe signal
//   - if resulting signal better than predicted, continue
//   - otherwise, ditch run, and don't update from it

// - one explore per safe signal

// - 200 sample train new very slow
//   - but not too bad actually?

// TODO: try with 10% of explore going through
// - true results better
// - actually not that bad

// - not exactly safe/unsafe
//   - but: what do you want to include as something to solve

// - even no save/explore always going through not that bad
//   - w/ result probably making things way too good
//     - or indicates how impactful signals can be
// - means clean not important besides explore/train new
//   - ramp either works regardless, or options that are best in damage get selected

// - can have clean vs. damage phase
//   - train existing signal on clean

// TODO: try not safe, but familiar
// - so while recent nodes are in unfamiliar spots, don't try/register stuff
// - but once back to familiar spots, begin trying things again

// - should one bad signal halt progress?
// - should one good signal restart it?

// - no exit makes progress difficult, even with perfect signal
//   - means lots of bad candidates are being produced
//     - what explore is training for is not similar to eval

// - just not meaningful to keep going after bad mistakes?
//   - better and efficient to simply give up and start anew
//     - but how to reach later half to experiment?
//       - need to practice/simulate?
//       - need later spot that is consistently reachable
//       - need world model to capture what later spot looks like
//       - need to find spot that matches conditions
//     - not achievable within the context of the problem?

// - though, if same scope is used at start and end...
//   - ...explore will be done on start, but eval will also apply to end

// - though if 50% of the time, running through fully for eval anyways...
//   - ...then not a big deal to run through first half to reach explore

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
#include "minesweeper.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
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
			// target_val -= 0.0001 * solution_wrapper->num_actions;

			solution_wrapper->experiment_end(target_val);

			delete problem;

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
