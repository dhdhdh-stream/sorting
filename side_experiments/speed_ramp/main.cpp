// TODO: improve explore quality
// - world modeling, etc.
// - result will probably also help long problems
// - actions include complex actions, and exiting to certain parts of the solution

// TODO: with GPU, don't use any libraries, but use CUDA directly

// TODO: try really cutting down on number of samples
// - really rely on rampup to make things safe

// - when combining, only try scopes that generalize

// TODO: save full runs and use to train existing instead of saving separate ScopeHistorys
// - maybe 4000 samples with 5% min hit ratio
//   - saves won't include explores
//   - calc and save signal val on exit scope
// - can also save vals, and retrain existing too
//   - maybe for each stage for the right contrast

// TODO: try using combined signal instead of just true
// - might be better now that combined always contains true

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
