// combinations of actions are orthogonal to world model, and both are needed
// - world model to eliminate redundant information
//   - may also guide exploration and decision making?
// - but actions may cause permanent changes to the world, and specific sequences of actions may be needed to reach good outcomes

// maybe after a pass, reverse engineer to determine what would happen if a different choice was made
// - use world model from the future to predict what would happen in the past

// perhaps mimicking is based off of world model
// - world model to make things Markov
//   - then can align someone else's actions
//   - learn Markov actions based on world model
//     - then when incorporating, check against learned model to see if something different should be done, and can explore down that path

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "abstract_experiment.h"
#include "abstract_node.h"
#include "constants.h"
#include "globals.h"
#include "minesweeper.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_set.h"

using namespace std;

int seed;

default_random_engine generator;

ProblemType* problem_type;
SolutionSet* solution_set;

int run_index;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	problem_type = new TypeMinesweeper();

	solution_set = new SolutionSet();
	// solution_set->init();
	solution_set->load("", "main");

	solution_set->increment();

	// solution_set->save("", "main");

	run_index = 0;

	while (true) {
		Problem* problem = problem_type->get_problem();

		RunHelper run_helper;

		#if defined(MDEBUG) && MDEBUG
		run_helper.starting_run_seed = run_index;
		run_helper.curr_run_seed = run_index;
		#endif /* MDEBUG */
		run_index++;
		if (run_index%10000 == 0) {
			cout << "run_index: " << run_index << endl;
		}

		run_helper.result = get_existing_result(problem);

		vector<ContextLayer> context;
		Solution* solution = solution_set->solutions[solution_set->curr_solution_index];
		solution->scopes[0]->activate(
			problem,
			context,
			run_helper);

		if (run_helper.experiments_seen_order.size() == 0) {
			if (!run_helper.exceeded_limit) {
				create_experiment(run_helper);
			}
		}

		double target_val;
		if (!run_helper.exceeded_limit) {
			target_val = problem->score_result(run_helper.num_analyze,
											   run_helper.num_actions);
		} else {
			target_val = -1.0;
		}

		delete problem;

		#if defined(MDEBUG) && MDEBUG
		if (run_index%2000 == 0) {
			delete solution_set;
			solution_set = new SolutionSet();
			solution_set->load("", "main");

			solution_set->increment();

			continue;
		}
		#endif /* MDEBUG */

		if (run_helper.experiment_histories.size() > 0) {
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
				SolutionSet* duplicate = new SolutionSet(solution_set);
				Solution* duplicate_solution = duplicate->solutions[duplicate->curr_solution_index];

				duplicate_solution->last_updated_scope_id = run_helper.experiment_histories.back()->experiment->scope_context->id;
				if (run_helper.experiment_histories.back()->experiment->type == EXPERIMENT_TYPE_NEW_ACTION) {
					duplicate_solution->last_new_scope_id = (int)duplicate_solution->scopes.size();
				}

				run_helper.experiment_histories.back()->experiment->finalize(duplicate_solution);
				delete run_helper.experiment_histories.back()->experiment;

				Scope* experiment_scope = duplicate_solution->scopes[duplicate_solution->last_updated_scope_id];
				clean_scope(experiment_scope);

				#if defined(MDEBUG) && MDEBUG
				while (duplicate_solution->verify_problems.size() > 0) {
					Problem* problem = duplicate_solution->verify_problems[0];

					RunHelper run_helper;
					run_helper.starting_run_seed = duplicate_solution->verify_seeds[0];
					cout << "run_helper.starting_run_seed: " << run_helper.starting_run_seed << endl;
					run_helper.curr_run_seed = duplicate_solution->verify_seeds[0];
					duplicate_solution->verify_seeds.erase(duplicate_solution->verify_seeds.begin());

					vector<ContextLayer> context;
					duplicate_solution->scopes[0]->verify_activate(
						problem,
						context,
						run_helper);

					cout << "run_helper.num_actions: " << run_helper.num_actions << endl;
					cout << "duplicate_solution->num_actions_limit: " << duplicate_solution->num_actions_limit << endl;

					delete duplicate_solution->verify_problems[0];
					duplicate_solution->verify_problems.erase(duplicate_solution->verify_problems.begin());
				}
				duplicate_solution->clear_verify();
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
					run_helper.curr_run_seed = run_index;
					run_index++;
					#endif /* MDEBUG */

					vector<ContextLayer> context;
					duplicate_solution->scopes[0]->measure_activate(
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

				for (int s_index = 0; s_index < (int)duplicate_solution->scopes.size(); s_index++) {
					for (map<int, AbstractNode*>::iterator it = duplicate_solution->scopes[s_index]->nodes.begin();
							it != duplicate_solution->scopes[s_index]->nodes.end(); it++) {
						it->second->average_instances_per_run /= MEASURE_ITERS;
						if (it->second->average_instances_per_run < 1.0) {
							it->second->average_instances_per_run = 1.0;
						}
					}
				}

				#if defined(MDEBUG) && MDEBUG
				if (early_exit) {
					delete duplicate;

					delete solution_set;
					solution_set = new SolutionSet();
					solution_set->load("", "main");

					solution_set->increment();

					continue;
				}
				#endif /* MDEBUG */

				double sum_score = 0.0;
				for (int d_index = 0; d_index < MEASURE_ITERS; d_index++) {
					sum_score += target_vals[d_index];
				}
				duplicate->average_score = sum_score / MEASURE_ITERS;

				duplicate_solution->max_num_actions = max_num_actions;
				duplicate_solution->num_actions_limit = 2*duplicate_solution->max_num_actions + 10;

				cout << "duplicate->average_score: " << duplicate->average_score << endl;

				ofstream display_file;
				display_file.open("../display.txt");
				duplicate_solution->save_for_display(display_file);
				display_file.close();

				duplicate->timestamp++;

				#if defined(MDEBUG) && MDEBUG
				delete solution_set;
				solution_set = duplicate;

				// solution_set->save("", "main");

				solution_set->increment();
				#else
				delete duplicate;
				#endif /* MDEBUG */
			}
		} else {
			for (int e_index = 0; e_index < (int)run_helper.experiments_seen_order.size(); e_index++) {
				AbstractExperiment* experiment = run_helper.experiments_seen_order[e_index];
				experiment->average_remaining_experiments_from_start =
					0.9 * experiment->average_remaining_experiments_from_start
					+ 0.1 * ((int)run_helper.experiments_seen_order.size()-1 - e_index);
			}
		}
	}

	delete problem_type;
	delete solution_set;

	cout << "Done" << endl;
}
