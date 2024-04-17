#include <iostream>

#include "globals.h"
#include "minesweeper.h"
#include "problem.h"
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

	{
		Minesweeper* problem = new Minesweeper();

		RunHelper run_helper;

		int num_steps = 0;
		int target_steps = 3 + solution->timestamp/2;

		while (true) {
			vector<ContextLayer> context;
			context.push_back(ContextLayer());

			context.back().scope = solution->scopes[0];
			context.back().node = NULL;

			ScopeHistory* root_history = new ScopeHistory(solution->scopes[0]);
			context.back().scope_history = root_history;

			// unused
			int exit_depth = -1;
			AbstractNode* exit_node = NULL;

			solution->scopes[0]->step_through_activate(
				problem,
				context,
				exit_depth,
				exit_node,
				run_helper,
				root_history);

			delete root_history;

			num_steps++;
			if (num_steps >= target_steps) {
				break;
			} else {
				bool should_continue = problem->travel();
				if (!should_continue) {
					break;
				}
			}
		}

		string input_gate;
		cin >> input_gate;

		problem->print();
		cout << "Done" << endl;
		double target_val;
		if (!run_helper.exceeded_limit) {
			target_val = problem->score_result();
		} else {
			target_val = -1.0;
		}
		cout << "target_val: " << target_val << endl;

		delete problem;
	}

	cout << "Seed: " << seed << endl;

	delete solution;

	cout << "Done" << endl;
}
