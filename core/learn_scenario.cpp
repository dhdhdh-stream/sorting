#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "globals.h"
#include "run_helper.h"
#include "solution.h"
#include "minesweeper_open.h"
#include "scenario_experiment.h"
#include "scope.h"

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
	solution->init();
	// solution->load("", "main");

	solution->save("", "main");

	ScenarioExperiment* experiment = new ScenarioExperiment(new MinesweeperOpen());

	while (true) {
		Scenario* scenario = new MinesweeperOpen();

		RunHelper run_helper;

		experiment->activate(scenario,
							 run_helper);

		bool is_sequence = scenario->should_perform_sequence();

		experiment->backprop(is_sequence);

		delete scenario;

		if (experiment->state == SCENARIO_EXPERIMENT_STATE_DONE) {
			break;
		}
	}

	delete experiment;

	// Scope* new_scope;
	// for (map<int, Scope*>::iterator it = solution->scopes.begin();
	// 		it != solution->scopes.end(); it++) {
	// 	if (it->second->name == "minesweeper_open") {
	// 		new_scope = it->second;
	// 	}
	// }
	// for (int iter_index = 0; iter_index < 10; iter_index++) {
	// 	Scenario* scenario = new MinesweeperOpen();

	// 	RunHelper run_helper;

	// 	vector<ContextLayer> context;
	// 	context.push_back(ContextLayer());

	// 	context.back().scope = new_scope;
	// 	context.back().node = NULL;

	// 	ScopeHistory* root_history = new ScopeHistory(new_scope);
	// 	context.back().scope_history = root_history;

	// 	// unused
	// 	int exit_depth = -1;
	// 	AbstractNode* exit_node = NULL;

	// 	new_scope->activate(scenario->problem,
	// 						context,
	// 						exit_depth,
	// 						exit_node,
	// 						run_helper,
	// 						root_history);

	// 	cout << "root_history->node_histories.size(): " << root_history->node_histories.size() << endl;

	// 	delete root_history;

	// 	bool is_sequence = scenario->should_perform_sequence();
	// 	cout << "is_sequence: " << is_sequence << endl;

	// 	delete scenario;
	// }

	delete solution;

	cout << "Done" << endl;
}
