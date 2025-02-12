#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "branch_experiment.h"
#include "commit_experiment.h"
#include "constants.h"
#include "minesweeper.h"
#include "new_scope_experiment.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"

// temp
#include "abstract_node.h"

using namespace std;

default_random_engine generator;

ProblemType* problem_type;
Solution* solution;

int run_index;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	// int seed = (unsigned)time(NULL);
	int seed = 1739311170;
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

	while (true) {
		/**
		 * - need to be selective
		 *   - otherwise, more likely to be meaningless improvements
		 *     - which leads to even more meaningless improvements when experimenting from
		 */
		BranchExperiment* best_experiment = NULL;
		int improvement_iter = 0;
		while (true) {
			run_index++;
			#if defined(MDEBUG) && MDEBUG
			if (run_index%10000 == 0) {
			#else
			if (run_index%100000 == 0) {
			#endif /* MDEBUG */
				cout << "run_index: " << run_index << endl;
				cout << "improvement_iter: " << improvement_iter << endl;
			}

			Problem* problem = problem_type->get_problem();

			RunHelper run_helper;

			ScopeHistory* scope_history = new ScopeHistory(solution->scopes[0]);
			solution->scopes[0]->activate(
					problem,
					run_helper,
					scope_history);

			double target_val = problem->score_result();

			// int expected_number_of_experiments = ceil((double)run_helper.num_actions / ACTIONS_PER_EXPERIMENT);
			// int expected_number_of_experiments = 2;
			int expected_number_of_experiments = 1;
			int number_of_experiments_diff = expected_number_of_experiments - (int)run_helper.experiments_seen.size();
			for (int e_index = 0; e_index < number_of_experiments_diff; e_index++) {
				create_experiment(scope_history);
			}

			delete scope_history;
			delete problem;

			set<AbstractExperiment*> experiments;
			for (int h_index = 0; h_index < (int)run_helper.experiment_histories.size(); h_index++) {
				run_helper.experiment_histories[h_index]->experiment->backprop(
					run_helper.experiment_histories[h_index],
					target_val);

				experiments.insert(run_helper.experiment_histories[h_index]->experiment);
			}

			for (set<AbstractExperiment*>::iterator it = experiments.begin();
					it != experiments.end(); it++) {
				AbstractExperiment* experiment = *it;
				experiment->update();
				if (experiment->result == EXPERIMENT_RESULT_FAIL) {
					cout << "EXPERIMENT_RESULT_FAIL" << endl;
					experiment->cleanup();
					delete experiment;
				} else if (experiment->result == EXPERIMENT_RESULT_SUCCESS) {
					experiment->cleanup();
					switch (experiment->type) {
					case EXPERIMENT_TYPE_BRANCH:
						{
							BranchExperiment* branch_experiment = (BranchExperiment*)experiment;
							if (best_experiment == NULL) {
								best_experiment = branch_experiment;
							} else {
								if (branch_experiment->improvement > best_experiment->improvement) {
									delete best_experiment;
									best_experiment = branch_experiment;
								} else {
									delete branch_experiment;
								}
							}
							improvement_iter++;
						}
						break;
					case EXPERIMENT_TYPE_NEW_SCOPE:
						if (solution->num_branch_experiments / BRANCH_PER_NEW_SCOPE
								> solution->num_new_scope_experiments) {
							experiment->add();
							solution->num_new_scope_experiments++;
						}
						delete experiment;
						break;
					case EXPERIMENT_TYPE_COMMIT:
						experiment->add();
						delete experiment;
						break;
					}
				}
			}
			if (improvement_iter >= IMPROVEMENTS_PER_ITER) {
				break;
			}
		}

		// temp
		cout << "selected BranchExperiment" << endl;
		cout << "best_experiment->scope_context->id: " << best_experiment->scope_context->id << endl;
		cout << "best_experiment->node_context->id: " << best_experiment->node_context->id << endl;
		cout << "best_experiment->is_branch: " << best_experiment->is_branch << endl;
		cout << "new explore path:";
		for (int s_index = 0; s_index < (int)best_experiment->best_step_types.size(); s_index++) {
			if (best_experiment->best_step_types[s_index] == STEP_TYPE_ACTION) {
				cout << " " << best_experiment->best_actions[s_index].move;
			} else {
				cout << " E" << best_experiment->best_scopes[s_index]->id;
			}
		}
		cout << endl;

		if (best_experiment->best_exit_next_node == NULL) {
			cout << "best_experiment->best_exit_next_node->id: " << -1 << endl;
		} else {
			cout << "best_experiment->best_exit_next_node->id: " << best_experiment->best_exit_next_node->id << endl;
		}

		cout << "best_experiment->new_average_score: " << best_experiment->new_average_score << endl;
		cout << "best_experiment->select_percentage: " << best_experiment->select_percentage << endl;

		cout << "best_experiment->improvement: " << best_experiment->improvement << endl;
		cout << "best_experiment->existing_average_score: " << best_experiment->existing_average_score << endl;

		cout << endl;

		best_experiment->add();
		delete best_experiment;

		solution->clean();

		// temp
		double sum_score = 0.0;
		for (int iter_index = 0; iter_index < 4000; iter_index++) {
			Problem* problem = problem_type->get_problem();

			RunHelper run_helper;

			ScopeHistory* scope_history = new ScopeHistory(solution->scopes[0]);
			solution->scopes[0]->measure_activate(
				problem,
				run_helper,
				scope_history);
			delete scope_history;

			double target_val = problem->score_result();
			sum_score += target_val;

			delete problem;
		}
		cout << "curr_score: " << sum_score / 4000 << endl;

		solution->num_branch_experiments++;

		solution->save("saves/", filename);

		ofstream display_file;
		display_file.open("../display.txt");
		solution->save_for_display(display_file);
		display_file.close();
	}

	delete problem_type;
	delete solution;

	cout << "Done" << endl;
}