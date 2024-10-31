/**
 * - world model is not about predictions
 *   - but simply about what is located where, and what is moving where
 *   - in humans, built naturally using optical flow/sensor fusion?
 * 
 * - world model enables localization
 *   - helps maintain identity/integrity of long solutions
 *     - breaks down solution into modular segments
 */

// TODO: to confirm if return node only useful for minesweeper in staying close to origin:
// - return node can only go to local location

// - can also try allowing return nodes only on scopes[0]
//   - or only on new scopes

// can also try trimming branches?
// - make later parts more consistent?

// try repeating scopes with no actions in between
// - try to force good coverage and consistency from them?

// move experiment selection and NewActionExperiment existing activation into result_activate

// then weigh NewActionExperiment towards repetition

// - for minesweeper, initially, only immediate local matters
//   - but after a point, global location in grid really matters
//     - how to go from not caring to caring?
// - in general, in the short term, merging speeds up progress
//   - e.g., finding the blank space in which can check for mines repeatedly
//   - but in the long term, things that don't matter in the short term begin to matter
//     - e.g., global position
//       - every decision from the entire solution matters
//         - perhaps try to use RL to crunch?

// - state needed for crunching down thousands of branch decisions
//   - e.g., initially, location doesn't matter
//     - by the time location matters, already hundreds of branches deep

// TODO: try giving location information

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "abstract_experiment.h"
#include "abstract_node.h"
#include "branch_experiment.h"
#include "constants.h"
#include "globals.h"
#include "minesweeper.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

int seed;

default_random_engine generator;

ProblemType* problem_type;
Solution* solution;

int run_index;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	problem_type = new TypeMinesweeper();

	solution = new Solution();
	solution->init();
	// solution->load("", "main");

	solution->save("", "main");

	run_index = 0;

	while (true) {
		run_index++;
		if (run_index%10000 == 0) {
			cout << "run_index: " << run_index << endl;
		}

		#if defined(MDEBUG) && MDEBUG
		if (run_index%2000 == 0) {
			delete solution;
			solution = new Solution();
			solution->load("", "main");

			continue;
		}
		#endif /* MDEBUG */

		Problem* problem = problem_type->get_problem();

		RunHelper run_helper;

		get_existing_result(problem,
							run_helper);

		if (run_helper.experiment_histories.size() > 0) {
			run_helper.exceeded_limit = false;

			run_helper.num_analyze = 0;
			run_helper.num_actions = 0;

			#if defined(MDEBUG) && MDEBUG
			run_helper.starting_run_seed = run_index;
			run_helper.curr_run_seed = xorshift(run_helper.starting_run_seed);
			#endif /* MDEBUG */

			vector<ContextLayer> context;
			solution->scopes[0]->activate(
				problem,
				context,
				run_helper);

			double target_val;
			if (!run_helper.exceeded_limit) {
				target_val = problem->score_result(run_helper.num_analyze,
												   run_helper.num_actions);
			} else {
				target_val = -1.0;
			}

			for (int e_index = 0; e_index < (int)run_helper.experiments_seen_order.size(); e_index++) {
				AbstractExperiment* experiment = run_helper.experiments_seen_order[e_index];
				experiment->average_remaining_experiments_from_start =
					0.9 * experiment->average_remaining_experiments_from_start
					+ 0.1 * ((int)run_helper.experiments_seen_order.size()-1 - e_index
						+ run_helper.experiment_histories[0]->experiment->average_remaining_experiments_from_start);
			}

			run_helper.experiment_histories.back()->experiment->backprop(
				target_val,
				run_helper);
			if (run_helper.experiment_histories.back()->experiment->result == EXPERIMENT_RESULT_FAIL) {
				run_helper.experiment_histories.back()->experiment->finalize(NULL);
				delete run_helper.experiment_histories.back()->experiment;
			} else if (run_helper.experiment_histories.back()->experiment->result == EXPERIMENT_RESULT_SUCCESS) {
				/**
				 * - history->experiment_histories.size() == 1
				 */
				Solution* duplicate = new Solution(solution);

				int last_updated_scope_id = run_helper.experiment_histories.back()->experiment->scope_context->id;

				run_helper.experiment_histories.back()->experiment->finalize(duplicate);
				delete run_helper.experiment_histories.back()->experiment;

				Scope* experiment_scope = duplicate->scopes[last_updated_scope_id];
				clean_scope(experiment_scope);
				duplicate->clean();

				#if defined(MDEBUG) && MDEBUG
				while (duplicate->verify_problems.size() > 0) {
					Problem* problem = duplicate->verify_problems[0];

					RunHelper run_helper;
					run_helper.starting_run_seed = duplicate->verify_seeds[0];
					cout << "run_helper.starting_run_seed: " << run_helper.starting_run_seed << endl;
					run_helper.curr_run_seed = xorshift(run_helper.starting_run_seed);
					duplicate->verify_seeds.erase(duplicate->verify_seeds.begin());

					vector<ContextLayer> context;
					duplicate->scopes[0]->verify_activate(
						problem,
						context,
						run_helper);

					cout << "run_helper.num_actions: " << run_helper.num_actions << endl;
					cout << "duplicate->num_actions_limit: " << duplicate->num_actions_limit << endl;

					delete duplicate->verify_problems[0];
					duplicate->verify_problems.erase(duplicate->verify_problems.begin());
				}
				duplicate->clear_verify();
				#endif /* MDEBUG */

				vector<double> target_vals;
				int max_num_actions = 0;
				#if defined(MDEBUG) && MDEBUG
				bool early_exit = false;
				#endif /* MDEBUG */
				for (int iter_index = 0; iter_index < MEASURE_ITERS; iter_index++) {
					Problem* problem = problem_type->get_problem();

					RunHelper run_helper;
					#if defined(MDEBUG) && MDEBUG
					run_helper.starting_run_seed = run_index;
					run_helper.curr_run_seed = xorshift(run_helper.starting_run_seed);
					run_index++;
					#endif /* MDEBUG */

					vector<ContextLayer> context;
					duplicate->scopes[0]->measure_activate(
						problem,
						context,
						run_helper);

					if (run_helper.num_actions > max_num_actions) {
						max_num_actions = run_helper.num_actions;
					}

					double target_val;
					if (!run_helper.exceeded_limit) {
						target_val = problem->score_result(run_helper.num_analyze,
														   run_helper.num_actions);
					} else {
						target_val = -1.0;
					}

					target_vals.push_back(target_val);

					delete problem;

					#if defined(MDEBUG) && MDEBUG
					if (run_index%2000 == 0) {
						early_exit = true;
						break;
					}
					#endif /* MDEBUG */
				}

				for (int s_index = 0; s_index < (int)duplicate->scopes.size(); s_index++) {
					for (map<int, AbstractNode*>::iterator it = duplicate->scopes[s_index]->nodes.begin();
							it != duplicate->scopes[s_index]->nodes.end(); it++) {
						it->second->average_instances_per_run /= MEASURE_ITERS;
					}
				}

				#if defined(MDEBUG) && MDEBUG
				if (early_exit) {
					delete duplicate;

					delete solution;
					solution = new Solution();
					solution->load("", "main");

					delete problem;

					continue;
				}
				#endif /* MDEBUG */

				double sum_score = 0.0;
				for (int d_index = 0; d_index < MEASURE_ITERS; d_index++) {
					sum_score += target_vals[d_index];
				}
				duplicate->average_score = sum_score / MEASURE_ITERS;

				duplicate->max_num_actions = max_num_actions;
				duplicate->num_actions_limit = 10*duplicate->max_num_actions + 10;

				cout << "duplicate->average_score: " << duplicate->average_score << endl;

				ofstream display_file;
				display_file.open("../display.txt");
				duplicate->save_for_display(display_file);
				display_file.close();

				duplicate->timestamp++;

				duplicate->update_subproblem();

				#if defined(MDEBUG) && MDEBUG
				delete solution;
				solution = duplicate;

				solution->save("", "main");
				#else
				delete duplicate;
				#endif /* MDEBUG */
			}
		}

		delete problem;
	}

	delete problem_type;
	delete solution;

	cout << "Done" << endl;
}
