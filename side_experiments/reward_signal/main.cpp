// TODO: hooks
// TODO: no seperate measure phase
// - just reuse experiment measure

// - signals may not be good because solution not consistent
//   - things get increasingly more mixed
// - probably don't pursue further until consistency addressed

// - standard deviation/leeway increases and increases

// - also probably not worth it over using score when single layer
//   - but may be useful with scopes involved
//   - e.g., (after - before) within scope correlated with final score, so instead maximize that for each instance

// - clean signals might also be hard to find because of factors tied to actions
//   - so correlated with what's been done, and not what's possible

// - try only merging paths if exactly identical
//   - if not exactly identical, then add extra state and capture exact differences

// - don't have to correlate against final score
//   - can correlate against layer above
//     - OK if not ancestor all the time, as once correlated, can use reward signal without outer moving forward

// - probably just train score function to each scope and optimize against it

// - OK to merge initially
//   - will create new mixed state, but still have non-mixed states

// - actually, shouldn't try to preserve existing state due to mixed state

// TODO: try to always reduce variance

// - don't worry about consistency, and just let scopes sand things into place

// - maybe reward signals useful for optimizing existing
//   - still need true signal for innovation

// - maybe save explore samples
//   - learned results will still be about maintaining existing

// TODO: experiment with ratios:
// - existing with good outcome
// - existing with neutral outcome
// - chaos

// train reward signal using previous samples

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
#include "simpler.h"
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

	ProblemType* problem_type = new TypeSimpler();

	string filename;
	SolutionWrapper* solution_wrapper;
	if (argc > 1) {
		filename = argv[1];
		solution_wrapper = new SolutionWrapper(
			problem_type->num_obs(),
			"saves/",
			filename);
	} else {
		filename = "main.txt";
		solution_wrapper = new SolutionWrapper(
			problem_type->num_obs());
	}

	#if defined(MDEBUG) && MDEBUG
	while (true) {
	#else
	// while (!solution_wrapper->is_done()) {
	while (true) {
	#endif /* MDEBUG */
		double sum_score = 0.0;
		for (int iter_index = 0; iter_index < MEASURE_ITERS; iter_index++) {
			Problem* problem = problem_type->get_problem();

			solution_wrapper->measure_init();

			while (true) {
				vector<double> obs = problem->get_observations();

				pair<bool,int> next = solution_wrapper->measure_step(obs);
				if (next.first) {
					break;
				} else {
					problem->perform_action(next.second);
				}
			}

			double target_val = problem->score_result();
			target_val -= 0.0001 * solution_wrapper->num_actions;
			sum_score += target_val;

			solution_wrapper->measure_end(target_val);

			delete problem;
		}

		double new_score = sum_score / (double)MEASURE_ITERS;
		cout << "new_score: " << new_score << endl;
		solution_wrapper->measure_update(new_score);

		solution_wrapper->save("saves/", filename);

		solution_wrapper->save_for_display("../", "display.txt");

		// #if defined(MDEBUG) && MDEBUG
		// delete solution_wrapper;
		// solution_wrapper = new SolutionWrapper(
		// 	problem_type->num_obs(),
		// 	"saves/",
		// 	filename);
		// #endif /* MDEBUG */

		while (true) {
			int starting_timestamp = solution_wrapper->solution->timestamp;

			Problem* problem = problem_type->get_problem();
			#if defined(MDEBUG) && MDEBUG
			solution_wrapper->problem = problem;
			#endif /* MDEBUG */

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

			if (solution_wrapper->solution->timestamp > starting_timestamp) {
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
	}

	solution_wrapper->clean_scopes();
	solution_wrapper->save("saves/", filename);

	delete problem_type;
	delete solution_wrapper;

	cout << "Done" << endl;
}
