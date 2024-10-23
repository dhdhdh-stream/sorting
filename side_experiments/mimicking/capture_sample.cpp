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
