#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "abstract_node.h"
#include "globals.h"
#include "helpers.h"
#include "scope.h"
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

	{
		Problem* problem = problem_type->get_problem();

		solution_wrapper->init();

		while (true) {
			vector<double> obs = problem->get_observations();

			pair<bool,int> next = solution_wrapper->step(obs);
			if (next.first) {
				break;
			} else {
				problem->perform_action(next.second);
			}
		}

		double target_val = problem->score_result();
		target_val -= 0.0001 * solution_wrapper->num_actions;
		cout << "target_val: " << target_val << endl;

		// temp
		ScopeHistory* scope_history = solution_wrapper->scope_histories[0];
		if (scope_history->node_histories.find(1748) != scope_history->node_histories.end()) {
			ScopeNode* scope_node = (ScopeNode*)solution_wrapper->solution->scopes[0]->nodes[1748];
			double signal = calc_signal(scope_node,
										scope_history);
			cout << "signal: " << signal << endl;

			Simpler* simpler = (Simpler*)problem;
			cout << "simpler->random_factor: " << simpler->random_factor << endl;
		}
		// TODO: when selecting factors for signal, don't depend on nodes in scope node

		// TODO: for Simpler, try only being able to insert at front
		// - swap to starting with 2 nodes, start node and end node
		//   - start node can never be a factor

		solution_wrapper->end();

		problem->print();

		cout << "solution_wrapper->num_actions: " << solution_wrapper->num_actions << endl;

		delete problem;
	}

	solution_wrapper->save_for_display("../", "display.txt");

	delete problem_type;
	delete solution_wrapper;

	cout << "Done" << endl;
}
