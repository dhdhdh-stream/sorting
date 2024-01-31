#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "globals.h"
#include "run_helper.h"
#include "solution.h"
#include "minesweeper.h"
#include "minesweeper_flag_remaining.h"
#include "minesweeper_open.h"
#include "minesweeper_open_remaining.h"
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

	// {
	// 	ScenarioExperiment* experiment = new ScenarioExperiment(new MinesweeperOpen());

	// 	while (true) {
	// 		Scenario* scenario = new MinesweeperOpen();

	// 		RunHelper run_helper;

	// 		experiment->activate(scenario,
	// 							 run_helper);

	// 		bool is_sequence = scenario->should_perform_sequence();

	// 		experiment->backprop(is_sequence);

	// 		delete scenario;

	// 		if (experiment->state == SCENARIO_EXPERIMENT_STATE_DONE) {
	// 			break;
	// 		}
	// 	}

	// 	delete experiment;
	// }

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

	// {
	// 	ScenarioExperiment* experiment = new ScenarioExperiment(new MinesweeperOpenRemaining());

	// 	while (true) {
	// 		Scenario* scenario = new MinesweeperOpenRemaining();

	// 		RunHelper run_helper;

	// 		experiment->activate(scenario,
	// 							 run_helper);

	// 		bool is_sequence = scenario->should_perform_sequence();

	// 		experiment->backprop(is_sequence);

	// 		delete scenario;

	// 		if (experiment->state == SCENARIO_EXPERIMENT_STATE_DONE) {
	// 			break;
	// 		}
	// 	}

	// 	delete experiment;
	// }

	// Scope* new_scope;
	// for (map<int, Scope*>::iterator it = solution->scopes.begin();
	// 		it != solution->scopes.end(); it++) {
	// 	if (it->second->name == "minesweeper_open_remaining") {
	// 		new_scope = it->second;
	// 	}
	// }
	// for (int iter_index = 0; iter_index < 20; iter_index++) {
	// 	Scenario* scenario = new MinesweeperOpenRemaining();

	// 	bool is_sequence = scenario->should_perform_sequence();
	// 	cout << "is_sequence: " << is_sequence << endl;

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

	// 	delete scenario;
	// }

	{
		ScenarioExperiment* experiment = new ScenarioExperiment(new MinesweeperFlagRemaining());

		while (true) {
			Scenario* scenario = new MinesweeperFlagRemaining();

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
	}

	Scope* new_scope;
	for (map<int, Scope*>::iterator it = solution->scopes.begin();
			it != solution->scopes.end(); it++) {
		if (it->second->name == "minesweeper_flag_remaining") {
			new_scope = it->second;
		}
	}
	for (int iter_index = 0; iter_index < 50; iter_index++) {
		Scenario* scenario = new MinesweeperFlagRemaining();

		bool is_sequence = scenario->should_perform_sequence();
		cout << "is_sequence: " << is_sequence << endl;

		{
			Minesweeper* minesweeper = (Minesweeper*)scenario->problem;

			int num_open = 0;
			int num_remaining = 0;

			if (!minesweeper->revealed[1][1]
					&& !minesweeper->flagged[1][1]) {
				num_open++;
			}
			if (minesweeper->world[1][1] == -1
					&& !minesweeper->flagged[1][1]) {
				num_remaining++;
			}

			if (!minesweeper->revealed[1][2]
					&& !minesweeper->flagged[1][2]) {
				num_open++;
			}
			if (minesweeper->world[1][2] == -1
					&& !minesweeper->flagged[1][2]) {
				num_remaining++;
			}

			if (!minesweeper->revealed[1][3]
					&& !minesweeper->flagged[1][3]) {
				num_open++;
			}
			if (minesweeper->world[1][3] == -1
					&& !minesweeper->flagged[1][3]) {
				num_remaining++;
			}

			if (!minesweeper->revealed[2][3]
					&& !minesweeper->flagged[2][3]) {
				num_open++;
			}
			if (minesweeper->world[2][3] == -1
					&& !minesweeper->flagged[2][3]) {
				num_remaining++;
			}

			if (!minesweeper->revealed[3][3]
					&& !minesweeper->flagged[3][3]) {
				num_open++;
			}
			if (minesweeper->world[3][3] == -1
					&& !minesweeper->flagged[3][3]) {
				num_remaining++;
			}

			if (!minesweeper->revealed[3][2]
					&& !minesweeper->flagged[3][2]) {
				num_open++;
			}
			if (minesweeper->world[3][2] == -1
					&& !minesweeper->flagged[3][2]) {
				num_remaining++;
			}

			if (!minesweeper->revealed[3][1]
					&& !minesweeper->flagged[3][1]) {
				num_open++;
			}
			if (minesweeper->world[3][1] == -1
					&& !minesweeper->flagged[3][1]) {
				num_remaining++;
			}

			if (!minesweeper->revealed[2][1]
					&& !minesweeper->flagged[2][1]) {
				num_open++;
			}
			if (minesweeper->world[2][1] == -1
					&& !minesweeper->flagged[2][1]) {
				num_remaining++;
			}

			cout << "num_open: " << num_open << endl;
			cout << "num_remaining: " << num_remaining << endl;
		}

		RunHelper run_helper;

		vector<ContextLayer> context;
		context.push_back(ContextLayer());

		context.back().scope = new_scope;
		context.back().node = NULL;

		ScopeHistory* root_history = new ScopeHistory(new_scope);
		context.back().scope_history = root_history;

		// unused
		int exit_depth = -1;
		AbstractNode* exit_node = NULL;

		new_scope->activate(scenario->problem,
							context,
							exit_depth,
							exit_node,
							run_helper,
							root_history);

		cout << "root_history->node_histories.size(): " << root_history->node_histories.size() << endl;

		delete root_history;

		delete scenario;
	}

	delete solution;

	cout << "Done" << endl;
}
