#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "helpers.h"
#include "simpler.h"
#include "scope.h"
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

	SolutionWrapper* solution_wrapper = new SolutionWrapper(
		problem_type->num_obs(),
		"saves/",
		"main.txt");

	Scope* scope = solution_wrapper->solution->scopes[0];

	Problem* problem = problem_type->get_problem();

	vector<vector<double>> pre_obs_history;
	vector<vector<double>> post_obs_history;

	pre_obs_history.push_back(problem->get_observations());
	for (int a_index = 0; a_index < (int)scope->signal_pre_actions.size(); a_index++) {
		problem->perform_action(scope->signal_pre_actions[a_index]);

		pre_obs_history.push_back(problem->get_observations());
	}

	geometric_distribution<int> num_random_distribution(0.05);
	uniform_int_distribution<int> action_distribution(0, 2);
	int num_random = num_random_distribution(generator);
	for (int a_index = 0; a_index < num_random; a_index++) {
		problem->perform_action(action_distribution(generator));
	}

	post_obs_history.push_back(problem->get_observations());
	for (int a_index = 0; a_index < (int)scope->signal_post_actions.size(); a_index++) {
		problem->perform_action(scope->signal_post_actions[a_index]);

		post_obs_history.push_back(problem->get_observations());
	}

	problem->print();

	double signal = calc_signal(pre_obs_history,
								post_obs_history,
								scope->signals,
								scope->miss_average_guess);
	cout << "signal: " << signal << endl;

	delete problem;

	delete problem_type;
	delete solution_wrapper;

	cout << "Done" << endl;
}
