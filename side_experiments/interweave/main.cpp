#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "abstract_experiment.h"
#include "constants.h"
#include "globals.h"
#include "minesweeper.h"
#include "problem.h"
#include "run_helper.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

default_random_engine generator;

ProblemType* problem_type;
Solution* solution;

int run_index;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
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

		// temp
		{
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
		}
	}

	{
		ofstream display_file;
		display_file.open("../display.txt");
		solution->save_for_display(display_file);
		display_file.close();
	}

	run_index = 0;

	while (true) {
		run_index++;
		#if defined(MDEBUG) && MDEBUG
		if (run_index%10000 == 0) {
		#else
		if (run_index%100000 == 0) {
		#endif /* MDEBUG */
			cout << "run_index: " << run_index << endl;
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
		int expected_number_of_experiments = 2;
		int number_of_experiments_diff = expected_number_of_experiments - (int)run_helper.overall_histories.size();
		for (int e_index = 0; e_index < number_of_experiments_diff; e_index++) {
			create_experiment(scope_history);
		}
		/**
		 * - more than expected will be created because experiments can skip over other experiments
		 */

		delete scope_history;
		delete problem;

		for (int h_index = 0; h_index < (int)run_helper.instance_histories.size(); h_index++) {
			run_helper.instance_histories[h_index]->experiment->backprop(
				run_helper.instance_histories[h_index],
				target_val);
		}

		bool reset = false;
		for (map<AbstractExperiment*, AbstractExperimentOverallHistory*>::iterator it = run_helper.overall_histories.begin();
				it != run_helper.overall_histories.end(); it++) {
			it->first->update(it->second,
							  target_val);
			if (it->first->result == EXPERIMENT_RESULT_FAIL) {
				// temp
				cout << "EXPERIMENT_RESULT_FAIL" << endl;
				it->first->finalize();
				delete it->first;
			} else if (it->first->result == EXPERIMENT_RESULT_SUCCESS) {
				it->first->finalize();
				delete it->first;

				// temp
				cout << "run_index: " << run_index << endl;
				{
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
				}

				solution->timestamp++;

				solution->save("saves/", filename);

				ofstream display_file;
				display_file.open("../display.txt");
				solution->save_for_display(display_file);
				display_file.close();

				if (solution->timestamp % SOLUTION_RESET_ITERS == 0) {
					reset = true;
				}
			}
		}

		if (reset) {
			solution->clean();

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
			solution->curr_score = sum_score / 4000;

			cout << "solution->curr_score: " << solution->curr_score << endl;
		}
	}

	delete problem_type;
	delete solution;

	cout << "Done" << endl;
};
