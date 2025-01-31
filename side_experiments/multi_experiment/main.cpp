/**
 * TODO: reset periodically
 */

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "abstract_experiment.h"
#include "constants.h"
#include "minesweeper.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

default_random_engine generator;

ProblemType* problem_type;
Solution* solution;

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
	}

	{
		ofstream display_file;
		display_file.open("../display.txt");
		solution->save_for_display(display_file);
		display_file.close();
	}

	while (true) {
		Problem* problem = problem_type->get_problem();

		RunHelper run_helper;

		ScopeHistory* scope_history = new ScopeHistory(solution->scopes[0]);
		solution->scopes[0]->activate(
				problem,
				run_helper,
				scope_history);

		double score = problem->score_result();
		double individual_impact = score / run_helper.num_actions;
		for (int h_index = 0; h_index < (int)run_helper.experiment_histories.size(); h_index++) {
			run_helper.experiment_histories[h_index]->impact += individual_impact;
		}

		int expected_number_of_experiments = ceil((double)run_helper.num_actions / ACTIONS_PER_EXPERIMENT);
		int number_of_experiments_diff = expected_number_of_experiments - run_helper.num_experiments_seen;
		for (int e_index = 0; e_index < number_of_experiments_diff; e_index++) {
			create_experiment(scope_history);
		}

		delete scope_history;
		delete problem;

		set<AbstractExperiment*> experiments;
		for (int h_index = 0; h_index < (int)run_helper.experiment_histories.size(); h_index++) {
			run_helper.experiment_histories[h_index]->experiment->backprop(
				run_helper.experiment_histories[h_index]);

			experiments.insert(run_helper.experiment_histories[h_index]->experiment);
		}

		set<Scope*> updated_scopes;
		for (set<AbstractExperiment*>::iterator it = experiments.begin();
				it != experiments.end(); it++) {
			AbstractExperiment* experiment = *it;
			experiment->update();
			if (experiment->result == EXPERIMENT_RESULT_FAIL) {
				experiment->finalize();
				delete experiment;
			} else {
				experiment->finalize();
				updated_scopes.insert(experiment->scope_context);
				delete experiment;
			}
		}
		if (updated_scopes.size() > 0) {
			for (set<Scope*>::iterator it = updated_scopes.begin();
					it != updated_scopes.end(); it++) {
				clean_scope(*it);
			}
			solution->clean_scopes();
		}
	}

	delete problem_type;
	delete solution;

	cout << "Done" << endl;
}