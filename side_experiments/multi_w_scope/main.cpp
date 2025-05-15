// TODO: add back repeat
// - new scope, but follow immediately with decision making

// TODO: for CommitExperiment, can separate commit from information gather?

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
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
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
	string filename;
	if (argc > 1) {
		filename = argv[1];
		solution->load("saves/", filename);
	} else {
		filename = "main.txt";
		solution->init();
		solution->save("saves/", filename);
	}

	{
		ofstream display_file;
		display_file.open("../display.txt");
		solution->save_for_display(display_file);
		display_file.close();
	}

	run_index = 0;

	while (solution->timestamp < EXPLORE_ITERS) {
		AbstractExperiment* best_experiment = NULL;

		int improvement_iter = 0;

		while (true) {
			run_index++;
			#if defined(MDEBUG) && MDEBUG
			if (run_index%1000 == 0) {
			#else
			if (run_index%100000 == 0) {
			#endif /* MDEBUG */
				cout << "run_index: " << run_index << endl;
				cout << "solution->timestamp: " << solution->timestamp << endl;
				cout << "improvement_iter: " << improvement_iter << endl;
			}

			Problem* problem = problem_type->get_problem();

			RunHelper run_helper;

			#if defined(MDEBUG) && MDEBUG
			run_helper.starting_run_seed = run_index;
			run_helper.curr_run_seed = xorshift(run_helper.starting_run_seed);
			#endif /* MDEBUG */

			ScopeHistory* scope_history = new ScopeHistory(solution->scopes[0]);
			solution->scopes[0]->experiment_activate(
					problem,
					run_helper,
					scope_history);

			double target_val = problem->score_result();

			bool is_return;
			Minesweeper* minesweeper = (Minesweeper*)problem;
			if (minesweeper->current_x == 4
					&& minesweeper->current_y == 4) {
				is_return = true;
			} else {
				is_return = false;
			}

			int expected_num_experiments = run_helper.num_original_actions / NODES_PER_EXPERIMENT;
			if (expected_num_experiments < 1) {
				expected_num_experiments = 1;
			}
			if (run_helper.num_experiment_instances < expected_num_experiments) {
				create_experiment(scope_history,
								  improvement_iter);
			}

			// temp
			if (run_index%100000 == 0) {
				cout << "run_helper.num_experiment_instances: " << run_helper.num_experiment_instances << endl;
			}

			delete scope_history;
			delete problem;

			for (map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it = run_helper.experiment_histories.begin();
					it != run_helper.experiment_histories.end(); it++) {
				it->first->backprop(target_val,
									is_return,
									run_helper);
				if (it->first->result == EXPERIMENT_RESULT_FAIL) {
					it->first->clean();
					delete it->first;
				} else if (it->first->result == EXPERIMENT_RESULT_SUCCESS) {
					it->first->clean();

					if (best_experiment == NULL) {
						best_experiment = it->first;
					} else {
						if (it->first->improvement > best_experiment->improvement) {
							delete best_experiment;
							best_experiment = it->first;
						} else {
							delete it->first;
						}
					}

					improvement_iter++;
				}
			}

			if (improvement_iter >= IMPROVEMENTS_PER_ITER) {
				break;
			}
		}

		Scope* last_updated_scope = best_experiment->scope_context;

		best_experiment->add();
		delete best_experiment;

		clean_scope(last_updated_scope);

		if (last_updated_scope->nodes.size() >= SCOPE_EXCEEDED_NUM_NODES) {
			last_updated_scope->exceeded = true;
		}
		if (last_updated_scope->nodes.size() <= SCOPE_RESUME_NUM_NODES) {
			last_updated_scope->exceeded = false;
		}

		solution->clean();

		double sum_score = 0.0;
		vector<ScopeHistory*> scope_histories;
		for (int iter_index = 0; iter_index < MEASURE_ITERS; iter_index++) {
			run_index++;

			Problem* problem = problem_type->get_problem();

			RunHelper run_helper;

			#if defined(MDEBUG) && MDEBUG
			run_helper.starting_run_seed = run_index;
			run_helper.curr_run_seed = xorshift(run_helper.starting_run_seed);
			#endif /* MDEBUG */

			ScopeHistory* scope_history = new ScopeHistory(solution->scopes[0]);
			solution->scopes[0]->measure_activate(
				problem,
				run_helper,
				scope_history);

			double target_val = problem->score_result();
			sum_score += target_val;

			scope_histories.push_back(scope_history);

			delete problem;
		}

		for (int h_index = 0; h_index < (int)scope_histories.size(); h_index++) {
			delete scope_histories[h_index];
		}

		double curr_drop = sum_score / MEASURE_ITERS - solution->curr_score;

		solution->curr_score = sum_score / MEASURE_ITERS;

		cout << "solution->curr_score: " << solution->curr_score << endl;

		if (curr_drop < solution->biggest_drop) {
			solution->biggest_drop = curr_drop;
		}

		solution->timestamp++;

		solution->save("saves/", filename);

		ofstream display_file;
		display_file.open("../display.txt");
		solution->save_for_display(display_file);
		display_file.close();

		#if defined(MDEBUG) && MDEBUG
		delete solution;
		solution = new Solution();
		solution->load("saves/", filename);
		#endif /* MDEBUG */
	}

	solution->clean_scopes();
	solution->save("saves/", filename);

	delete problem_type;
	delete solution;

	cout << "Done" << endl;
}
