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

default_random_engine generator;

Problem* problem_type;
Solution* solution;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	// problem_type = new Sorting();
	problem_type = new Minesweeper();

	solution = new Solution();
	solution->load("", "main");

	solution->curr_num_datapoints = 4000;

	ifstream suggestion_input_file;
	suggestion_input_file.open("suggestion_input.txt");
	AbstractExperiment* experiment = create_experiment(suggestion_input_file);
	suggestion_input_file.close();

	while (true) {
		// Problem* problem = new Sorting();
		Problem* problem = new Minesweeper();

		RunHelper run_helper;

		uniform_int_distribution<int> retry_distribution(0, 1);
		run_helper.should_restart = true;

		vector<ScopeHistory*> root_histories;
		while (run_helper.should_restart) {
			run_helper.should_restart = false;

			vector<ContextLayer> context;
			context.push_back(ContextLayer());

			context.back().scope = solution->root;
			context.back().node = NULL;

			ScopeHistory* root_history = new ScopeHistory(solution->root);
			context.back().scope_history = root_history;

			// unused
			int exit_depth = -1;
			AbstractNode* exit_node = NULL;

			solution->root->activate(problem,
									 context,
									 exit_depth,
									 exit_node,
									 run_helper,
									 root_history);

			root_histories.push_back(root_history);
		}

		for (int h_index = 0; h_index < (int)root_histories.size(); h_index++) {
			delete root_histories[h_index];
		}

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
				experiment->finalize();
				delete experiment;
				break;
			} else if (experiment->result == EXPERIMENT_RESULT_SUCCESS) {
				experiment->finalize();
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
