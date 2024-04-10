#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "abstract_experiment.h"
#include "action_node.h"
#include "globals.h"
#include "minesweeper.h"
#include "sorting.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"
#include "sorting.h"

using namespace std;

int seed;

default_random_engine generator;

Problem* problem_type;
Solution* solution;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	// problem_type = new Sorting();
	problem_type = new Minesweeper();

	solution = new Solution();
	solution->load("", "main");

	ifstream suggestion_input_file;
	suggestion_input_file.open("suggestion_input.txt");
	AbstractExperiment* experiment = create_experiment(suggestion_input_file);
	suggestion_input_file.close();

	while (true) {
		// Problem* problem = new Sorting();
		Problem* problem = new Minesweeper();

		RunHelper run_helper;

		vector<ContextLayer> context;
		context.push_back(ContextLayer());

		context.back().scope = solution->scopes[solution->curr_scope_id];
		context.back().node = NULL;

		ScopeHistory* root_history = new ScopeHistory(solution->scopes[solution->curr_scope_id]);
		context.back().scope_history = root_history;

		// unused
		int exit_depth = -1;
		AbstractNode* exit_node = NULL;

		solution->scopes[solution->curr_scope_id]->activate(
			solution->scopes[solution->curr_scope_id]->default_starting_node,
			problem,
			context,
			exit_depth,
			exit_node,
			run_helper,
			root_history);

		delete root_history;

		double target_val;
		if (!run_helper.exceeded_limit) {
			target_val = problem->score_result();
		} else {
			target_val = -1.0;
		}

		if (run_helper.experiment_histories.size() > 0) {
			run_helper.experiment_histories.back()->experiment->backprop(
				target_val,
				run_helper);
			if (experiment->result == EXPERIMENT_RESULT_FAIL) {
				Solution* empty = NULL;
				experiment->finalize(empty);
				delete experiment;
				break;
			} else if (experiment->result == EXPERIMENT_RESULT_SUCCESS) {
				Solution* duplicate = new Solution();
				duplicate->load("", "main");
				experiment->finalize(duplicate);
				delete experiment;

				solution->timestamp = (unsigned)time(NULL);
				solution->save("", "main");

				ofstream display_file;
				display_file.open("../display.txt");
				solution->save_for_display(display_file);
				display_file.close();

				break;
			}
		}

		delete problem;
	}

	delete problem_type;
	delete solution;

	cout << "Done" << endl;
}
