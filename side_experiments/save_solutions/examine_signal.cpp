#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "abstract_node.h"
#include "globals.h"
#include "helpers.h"
#include "scope.h"
#include "signal.h"
#include "simpler.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

int seed;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	ProblemType* problem_type = new TypeSimpler();

	string filename;
	SolutionWrapper* solution_wrapper;
	if (argc > 1) {
		filename = argv[1];
	} else {
		filename = "main.txt";
	}
	solution_wrapper = new SolutionWrapper(
		problem_type->num_obs(),
		"saves/",
		filename);

	Signal* signal = solution_wrapper->signals[0];

	ScopeHistory* scope_history = new ScopeHistory(solution_wrapper->solution->scopes[0]);

	Problem* problem = problem_type->get_problem();

	scope_history->signal_pre_obs.push_back(problem->get_observations());
	for (int a_index = 0; a_index < (int)signal->signal_pre_actions.size(); a_index++) {
		problem->perform_action(signal->signal_pre_actions[a_index]);

		scope_history->signal_pre_obs.push_back(problem->get_observations());
	}

	while (true) {
		problem->print();

		cout << "Input:" << endl;

		int action;
		cin >> action;
		problem->perform_action(action);
	}

	scope_history->signal_post_obs.push_back(problem->get_observations());
	for (int a_index = 0; a_index < (int)signal->signal_post_actions.size(); a_index++) {
		problem->perform_action(signal->signal_post_actions[a_index]);

		scope_history->signal_post_obs.push_back(problem->get_observations());
	}

	problem->print();

	double signal_val = calc_signal(scope_history,
									solution_wrapper);

	cout << "signal_val: " << signal_val << endl;

	delete problem;

	delete problem_type;
	delete solution_wrapper;

	cout << "Done" << endl;
}
