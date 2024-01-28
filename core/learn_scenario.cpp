#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "globals.h"
#include "run_helper.h"
#include "solution.h"
#include "minesweeper_remaining_mines.h"
#include "state_scenario_experiment.h"

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

	StateScenario* scenario = new MinesweeperRemainingMines();
	StateScenarioExperiment* experiment = new StateScenarioExperiment(scenario);
	delete scenario;

	while (true) {
		StateScenario* scenario = new MinesweeperRemainingMines();

		RunHelper run_helper;

		experiment->activate(scenario,
							 run_helper);

		double target_state = scenario->get_target_state();

		experiment->backprop(target_state);

		delete scenario;

		if (experiment->state == STATE_SCENARIO_EXPERIMENT_STATE_DONE) {
			break;
		}
	}

	delete experiment;

	delete solution;

	cout << "Done" << endl;
}
