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
		AbstractExperiment* curr_experiment = NULL;
		AbstractExperiment* best_experiment = NULL;

		int improvement_iter = 0;

		while (true) {
			run_index++;
			if (run_index%100000 == 0) {
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

			result_helper(problem,
						  run_helper,
						  curr_experiment);

			if (run_helper.experiment_history != NULL) {
				#if defined(MDEBUG) && MDEBUG
				run_helper.starting_run_seed = run_index;
				run_helper.curr_run_seed = xorshift(run_helper.starting_run_seed);
				#endif /* MDEBUG */

				ScopeHistory* scope_history = new ScopeHistory(solution->scopes[0]);
				solution->scopes[0]->experiment_activate(
						problem,
						run_helper,
						scope_history);
				delete scope_history;

				double target_val = problem->score_result();
				// target_val -= run_helper.num_actions * solution->curr_time_penalty;
				target_val -= run_helper.num_actions * 0.0001;

				run_helper.experiment_history->experiment->backprop(
					target_val,
					run_helper);
				if (run_helper.experiment_history->experiment->result == EXPERIMENT_RESULT_FAIL) {
					run_helper.experiment_history->experiment->clean();
					delete run_helper.experiment_history->experiment;

					curr_experiment = NULL;
				} else if (run_helper.experiment_history->experiment->result == EXPERIMENT_RESULT_SUCCESS) {
					run_helper.experiment_history->experiment->clean();

					if (best_experiment == NULL) {
						best_experiment = run_helper.experiment_history->experiment;
					} else {
						if (run_helper.experiment_history->experiment->improvement > best_experiment->improvement) {
							delete best_experiment;
							best_experiment = run_helper.experiment_history->experiment;
						} else {
							delete run_helper.experiment_history->experiment;
						}
					}

					curr_experiment = NULL;

					improvement_iter++;
				}
			}

			delete problem;

			if (improvement_iter >= IMPROVEMENTS_PER_ITER) {
				break;
			}
		}

		Scope* last_updated_scope = best_experiment->scope_context;

		best_experiment->add();
		delete best_experiment;

		clean_scope(last_updated_scope);

		#if defined(MDEBUG) && MDEBUG
		while (solution->verify_problems.size() > 0) {
			Problem* problem = solution->verify_problems[0];

			RunHelper run_helper;
			run_helper.starting_run_seed = solution->verify_seeds[0];
			cout << "run_helper.starting_run_seed: " << run_helper.starting_run_seed << endl;
			run_helper.curr_run_seed = xorshift(run_helper.starting_run_seed);
			solution->verify_seeds.erase(solution->verify_seeds.begin());

			ScopeHistory* scope_history = new ScopeHistory(solution->scopes[0]);
			solution->scopes[0]->verify_activate(
				problem,
				run_helper,
				scope_history);
			delete scope_history;

			delete solution->verify_problems[0];
			solution->verify_problems.erase(solution->verify_problems.begin());
		}
		solution->clear_verify();
		#endif /* MDEBUG */

		solution->clean();

		if (last_updated_scope->nodes.size() >= SCOPE_EXCEEDED_NUM_NODES) {
			last_updated_scope->exceeded = true;

			check_generalize(last_updated_scope);
		}
		if (last_updated_scope->nodes.size() <= SCOPE_RESUME_NUM_NODES) {
			last_updated_scope->exceeded = false;
		}

		double sum_score = 0.0;
		double sum_true_score = 0.0;
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
			// sum_score += target_val - run_helper.num_actions * solution->curr_time_penalty;
			sum_score += target_val - run_helper.num_actions * 0.0001;
			sum_true_score += target_val;

			update_scores(scope_history,
						  target_val);

			scope_histories.push_back(scope_history);

			delete problem;
		}

		solution->measure_update();

		update_matches(scope_histories);

		for (int h_index = 0; h_index < (int)scope_histories.size(); h_index++) {
			delete scope_histories[h_index];
		}

		solution->curr_score = sum_score / MEASURE_ITERS;
		solution->curr_true_score = sum_true_score / MEASURE_ITERS;

		cout << "solution->curr_score: " << solution->curr_score << endl;

		solution->timestamp++;

		if (solution->timestamp % INCREASE_TIME_PENALTY_ITER == 0) {
			solution->curr_time_penalty *= 1.25;
		}
		if (solution->curr_true_score > solution->best_true_score) {
			solution->best_true_score = solution->curr_true_score;
			solution->best_true_score_timestamp = solution->timestamp;
		}
		if (solution->best_true_score_timestamp < solution->timestamp
				&& (solution->timestamp - solution->best_true_score_timestamp)
					% DECREASE_TIME_PENALTY_ITER == 0) {
			solution->curr_time_penalty *= 0.8;
		}

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
