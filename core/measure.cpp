#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "globals.h"
#include "minesweeper.h"
#include "scope.h"
#include "solution.h"
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

	solution = new Solution();
	solution->load("", "main");

	double sum_vals = 0.0;
	Metrics metrics;

	Scope* starting_scope = solution->scopes[solution->curr_scope_id];
	// Scope* starting_scope = solution->scopes[6];

	cout << "starting_scope->layer: " << starting_scope->layer << endl;
	cout << "starting_scope->parent_id: " << starting_scope->parent_id << endl;
	cout << "starting_scope->child_ids.size(): " << starting_scope->child_ids.size() << endl;
	cout << "starting_scope->num_improvements: " << starting_scope->num_improvements << endl;

	for (int i_index = 0; i_index < 2000; i_index++) {
		// Problem* problem = new Sorting();
		Problem* problem = new Minesweeper();

		RunHelper run_helper;

		vector<ContextLayer> context;
		context.push_back(ContextLayer());

		context.back().scope = starting_scope;
		context.back().node = NULL;

		ScopeHistory* root_history = new ScopeHistory(starting_scope);
		context.back().scope_history = root_history;

		// unused
		int exit_depth = -1;
		AbstractNode* exit_node = NULL;

		starting_scope->measure_activate(
			starting_scope->default_starting_node,
			problem,
			context,
			exit_depth,
			exit_node,
			run_helper,
			metrics,
			root_history);

		delete root_history;

		double target_val;
		if (!run_helper.exceeded_limit) {
			target_val = problem->score_result();
		} else {
			target_val = -1.0;
		}
		sum_vals += target_val;

		delete problem;
	}

	for (map<Scope*, int>::iterator it = metrics.scope_counts.begin();
			it != metrics.scope_counts.end(); it++) {
		cout << it->first->id << ": " << it->second << endl;
	}

	cout << "average score: " << sum_vals/2000 << endl;

	ofstream display_file;
	display_file.open("../display.txt");
	solution->save_for_display(display_file);
	display_file.close();

	delete solution;

	cout << "Done" << endl;
}
