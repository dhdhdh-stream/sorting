#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

using namespace std;

default_random_engine generator;

Solution* solution;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	solution = new Solution();
	solution->load("", "main");

	{
		Problem* problem = new Sorting();
		// Problem* problem = new Minesweeper();

		RunHelper run_helper;

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

		delete root_history;

		double target_val;
		if (!run_helper.exceeded_limit) {
			target_val = problem->score_result();
		} else {
			target_val = -1.0;
		}
		cout << "target_val: " << target_val << endl;

		problem->print();

		delete problem;
	}

	ofstream display_file;
	display_file.open("../display.txt");
	solution->save_for_display(display_file);
	display_file.close();

	delete solution;

	cout << "Done" << endl;
}
