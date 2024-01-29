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

	Scenario* scenario = new MinesweeperOpen();
	ScenarioExperiment* experiment = new ScenarioExperiment(scenario);
	delete scenario;

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

	delete solution;

	cout << "Done" << endl;
}
