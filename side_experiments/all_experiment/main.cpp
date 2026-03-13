// - unstable
//   - deleted something useful
//     - that was sharp? and/or moved into sharp territory?

// - big gap between experiment scores and clean scores anyways
//   - doubt any measurements can be that great

// - randomness itself makes sharp solutions bad

// - there simply has to be potentially infinite experiments per run
//   - so the key must be about controlling the temperature
//     - experiments must be kept at a level in which solution can learn to recover?

// - maybe measure how much experiments are damaging for each part
//   - for parts where experiments are especially damaging, lower temperature
// TODO; measure separately
// - messy to do as part of solution building because experiments have fixed endpoint
// - for safest spot, set to 1/10, everywhere else, scale accordingly

// - solutions built under sharpness don't work when there's a lot of uncertainty

// - maybe being precise has a higher ceiling? but expensive to iterate on?
//   - and good to have at least a bit of robustness that randomness drives
//     - likely helps generalization

// - maybe simply have to live with randomness
//  - not that different from actions having random chance to fail anyways
//  - maybe resulting solution will be extremely robust
//    - which likely means it generalizes extremely well

// - maybe tied to how much effort needed to make safe
//   - if uncertainty is so much that even trying to make safe is unsafe, then impossible to progress

// - temperature tied to difficulty of the problem?
//   - mainly tied to how low temperature needs to be for saves to work

// - new scopes extremely damaging
// TODO: try with no new scopes? except from outer?

// - maybe new scopes try single

// TODO: measure damage of reusing scopes

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
			target_val -= 0.0001 * solution_wrapper->num_actions;

			solution_wrapper->experiment_end(target_val);

			delete problem;

			if (solution_wrapper->iter%100000 == 0) {
				cout << "solution_wrapper->iter: " << solution_wrapper->iter << endl;
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
