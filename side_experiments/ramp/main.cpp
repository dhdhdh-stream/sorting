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

	int sum_num_actions = 0;
	int sum_num_experiment_instances = 0;
	while (true) {
		run_index++;
		if (run_index%100000 == 0) {
			cout << "run_index: " << run_index << endl;
			cout << "solution->timestamp: " << solution->timestamp << endl;
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

		// temp
		if (run_index%100000 == 0) {
			double num_actions = (double)sum_num_actions / (double)CHECK_EXPERIMENT_ITER;
			double num_experiments = (double)sum_num_experiment_instances / (double)CHECK_EXPERIMENT_ITER;
			cout << "num_actions: " << num_actions << endl;
			cout << "num_experiments: " << num_experiments << endl;
		}

		sum_num_actions += run_helper.num_actions;
		sum_num_experiment_instances += run_helper.num_experiment_instances;
		if (run_index % CHECK_EXPERIMENT_ITER == 0) {
			double num_actions = (double)sum_num_actions / (double)CHECK_EXPERIMENT_ITER;
			double num_experiments = (double)sum_num_experiment_instances / (double)CHECK_EXPERIMENT_ITER;

			if (num_actions / (double)ACTIONS_PER_EXPERIMENT > num_experiments) {
				create_experiment(scope_history);
			}

			sum_num_actions = 0;
			sum_num_experiment_instances = 0;
		}

		delete scope_history;
		delete problem;

		set<Scope*> updated_scopes;
		for (map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it = run_helper.experiment_histories.begin();
				it != run_helper.experiment_histories.end(); it++) {
			it->first->backprop(target_val,
								it->second,
								updated_scopes);
		}
		for (int s_index = 0; s_index < (int)solution->scopes.size(); s_index++) {
			solution->scopes[s_index]->check_new_scope(updated_scopes);
		}
		if (updated_scopes.size() > 0) {
			for (set<Scope*>::iterator it = updated_scopes.begin();
					it != updated_scopes.end(); it++) {
				clean_scope(*it);

				if ((*it)->nodes.size() >= SCOPE_EXCEEDED_NUM_NODES) {
					(*it)->exceeded = true;

					check_generalize(*it);
				}
				if ((*it)->nodes.size() <= SCOPE_RESUME_NUM_NODES) {
					(*it)->exceeded = false;
				}
			}

			double sum_score = 0.0;
			for (int iter_index = 0; iter_index < MEASURE_ITERS; iter_index++) {
				run_index++;

				Problem* problem = problem_type->get_problem();

				RunHelper run_helper;

				#if defined(MDEBUG) && MDEBUG
				run_helper.starting_run_seed = run_index;
				run_helper.curr_run_seed = xorshift(run_helper.starting_run_seed);
				#endif /* MDEBUG */

				ScopeHistory* scope_history = new ScopeHistory(solution->scopes[0]);
				solution->scopes[0]->activate(
					problem,
					run_helper,
					scope_history);
				delete scope_history;

				double target_val = problem->score_result();
				sum_score += target_val;

				delete problem;
			}

			// temp
			double curr_drop = sum_score / MEASURE_ITERS - solution->curr_score;
			if (curr_drop < solution->biggest_drop) {
				solution->biggest_drop = curr_drop;
			}

			solution->curr_score = sum_score / MEASURE_ITERS;

			cout << "solution->curr_score: " << solution->curr_score << endl;

			solution->timestamp++;

			solution->save("saves/", filename);

			ofstream display_file;
			display_file.open("../display.txt");
			solution->save_for_display(display_file);
			display_file.close();
		}
	}

	solution->clean_scopes();
	solution->save("saves/", filename);

	delete problem_type;
	delete solution;

	cout << "Done" << endl;
}
