#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "abstract_experiment.h"
#include "globals.h"
#include "minesweeper.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

int seed;

default_random_engine generator;

ProblemType* problem_type;
Solution* solution;

int run_index;

const int BRANCH_FACTOR = 4;

int main(int argc, char* argv[]) {
	if (argc != 1 + BRANCH_FACTOR + 1) {
		cout << "Usage: ./combine [child ... files] [output]" << endl;
		exit(1);
	}
	
	vector<string> child_files(BRANCH_FACTOR);
	for (int c_index = 0; c_index < BRANCH_FACTOR; c_index++) {
		child_files[c_index] = argv[1 + c_index];
	}
	string output_file = argv[1 + BRANCH_FACTOR];

	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	problem_type = new TypeMinesweeper();

	solution = new Solution();
	solution->init();

	for (int existing_index = 0; existing_index < BRANCH_FACTOR; existing_index++) {
		Solution* existing_solution = new Solution();
		existing_solution->load("saves/", child_files[existing_index]);

		for (int scope_index = 1; scope_index < (int)existing_solution->scopes.size(); scope_index++) {
			solution->scopes.push_back(existing_solution->scopes[scope_index]);
		}

		existing_solution->scopes.erase(existing_solution->scopes.begin() + 1, existing_solution->scopes.end());

		delete existing_solution;
	}

	for (int scope_index = 1; scope_index < (int)solution->scopes.size(); scope_index++) {
		solution->scopes[scope_index]->id = scope_index;
	}

	for (int scope_index = 1; scope_index < (int)solution->scopes.size(); scope_index++) {
		solution->scopes[0]->existing_scopes.push_back(solution->scopes[scope_index]);
	}

	solution->num_existing_scopes = (int)solution->scopes.size() - 1;

	while (true) {
		Problem* problem = problem_type->get_problem();

		RunHelper run_helper;

		vector<ContextLayer> context;
		ScopeHistory* scope_history = new ScopeHistory(solution->scopes[0]);
		solution->scopes[0]->experiment_activate(
				problem,
				context,
				run_helper,
				scope_history);

		double target_val = problem->score_result();
		target_val -= 0.05 * run_helper.num_actions * solution->curr_time_penalty;
		target_val -= run_helper.num_analyze * solution->curr_time_penalty;

		if (run_helper.experiments_seen_order.size() == 0) {
			create_commit_experiment(scope_history);
		}

		delete scope_history;
		delete problem;

		if (run_helper.experiment_history != NULL) {
			for (int e_index = 0; e_index < (int)run_helper.experiments_seen_order.size(); e_index++) {
				AbstractExperiment* experiment = run_helper.experiments_seen_order[e_index];
				experiment->average_remaining_experiments_from_start =
					0.9 * experiment->average_remaining_experiments_from_start
					+ 0.1 * ((int)run_helper.experiments_seen_order.size()-1 - e_index
						+ run_helper.experiment_history->experiment->average_remaining_experiments_from_start);
			}

			run_helper.experiment_history->experiment->backprop(
				target_val,
				run_helper);
			if (run_helper.experiment_history->experiment->result == EXPERIMENT_RESULT_FAIL) {
				run_helper.experiment_history->experiment->finalize(NULL);
				delete run_helper.experiment_history->experiment;
			} else if (run_helper.experiment_history->experiment->result == EXPERIMENT_RESULT_SUCCESS) {
				int last_updated_scope_id = run_helper.experiment_history->experiment->scope_context->id;

				run_helper.experiment_history->experiment->finalize(solution);
				delete run_helper.experiment_history->experiment;

				Scope* experiment_scope = solution->scopes[last_updated_scope_id];
				clean_scope(experiment_scope,
							solution);
				solution->clean_scopes();

				break;
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

	solution->save("saves/", output_file);

	delete problem_type;
	delete solution;

	cout << "Done" << endl;
}
