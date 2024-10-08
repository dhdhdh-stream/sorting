/**
 * - needs to succeed with <200 samples(?)
 * 
 * - 2 scenarios:
 *   - copying someone else's strategy from scratch
 *     - through not really a scenario because will quickly become different and become 2nd case?
 *   - using someone else's strategy to improve what you already have
 * 
 * - need to first break down samples into compound actions?
 *   - can this be done in 200 samples?
 * - then can begin figuring out what follows what, when there are options/decisions to be made
 * 
 * - remember the beginning and end of every compound action in sample
 *   - remember e.g., their location
 *   - search existing solution for locations that are similar, and try
 * 
 * - segment by location?
 *   - either moving from point A to point B, or staying near a location
 *     - so points and lines
 *       - or just lines
 *     - then minimize distance from point or where should be on line
 * 
 * - if location doesn't move, then executing a sequence at a location
 *   - might be something copyable
 * 
 * - also use string distance to try to create repeats
 */

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "globals.h"
#include "minesweeper.h"
#include "problem.h"
#include "sample.h"

using namespace std;

int seed;

default_random_engine generator;

ProblemType* problem_type;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	problem_type = new TypeMinesweeper();

	Problem* problem = problem_type->get_problem();

	Sample sample;
	string id_input;
	cout << "id:" << endl;
	cin >> id_input;
	sample.id = stoi(id_input);

	sample.actions.push_back(Action(ACTION_NOOP));

	while (true) {
		vector<double> obs;
		vector<vector<int>> locations;
		problem->get_observations(obs,
								  locations);
		sample.obs.push_back(obs);
		sample.locations.push_back(locations);

		problem->print_obs();

		cout << "input:" << endl;

		string input;
		cin >> input;

		if (input.compare("x") == 0) {
			break;
		} else {
			int move = stoi(input);
			sample.actions.push_back(Action(move));
			problem->perform_action(Action(move));
		}
	}

	double target_val = problem->score_result(0, 0);
	sample.result = target_val;

	cout << "target_val: " << target_val << endl;

	sample.save();

	delete problem;

	delete problem_type;

	cout << "Done" << endl;
}
