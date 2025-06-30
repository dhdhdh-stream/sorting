#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "abstract_experiment.h"
#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "minesweeper.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

int seed;

default_random_engine generator;

int main(int argc, char* argv[]) {
	if (argc != 3) {
		cout << "Usage: ./worker [path] [filename]" << endl;
		exit(1);
	}
	string path = argv[1];
	string filename = argv[2];

	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	ProblemType* problem_type = new TypeMinesweeper();

	SolutionWrapper* solution_wrapper = new SolutionWrapper(
		problem_type->num_obs(), path, filename);

	auto start_time = chrono::high_resolution_clock::now();

	for (int iter_index = 0; iter_index < MEASURE_ITERS; iter_index++) {
		auto curr_time = chrono::high_resolution_clock::now();
		auto time_diff = chrono::duration_cast<chrono::seconds>(curr_time - start_time);
		if (time_diff.count() >= 20) {
			start_time = curr_time;

			cout << "solution_wrapper->improvement_iter: " << solution_wrapper->improvement_iter << endl;
		}

		Problem* problem = problem_type->get_problem();

		solution_wrapper->measure_init();

		while (true) {
			vector<double> obs = problem->get_observations();

			pair<bool,int> next = solution_wrapper->measure_step(obs);
			if (next.first) {
				break;
			} else {
				problem->perform_action(next.second);
			}
		}

		double target_val = problem->score_result();
		target_val -= 0.0001 * solution_wrapper->num_actions;

		solution_wrapper->measure_end(target_val);

		delete problem;
	}
	solution_wrapper->measure_update();

	while (!solution_wrapper->is_done()) {
		auto curr_time = chrono::high_resolution_clock::now();
		auto time_diff = chrono::duration_cast<chrono::seconds>(curr_time - start_time);
		if (time_diff.count() >= 20) {
			start_time = curr_time;

			cout << "solution_wrapper->improvement_iter: " << solution_wrapper->improvement_iter << endl;
		}

		int starting_timestamp = solution_wrapper->solution->timestamp;

		Problem* problem = problem_type->get_problem();

		solution_wrapper->experiment_init();

		while (true) {
			vector<double> obs = problem->get_observations();

			tuple<bool,bool,int> next = solution_wrapper->experiment_step(obs);
			if (get<0>(next)) {
				break;
			} else if (get<1>(next)) {
				uniform_int_distribution<int> action_distribution(0, problem_type->num_possible_actions()-1);
				int new_action = action_distribution(generator);

				solution_wrapper->set_action(new_action);

				problem->perform_action(new_action);
			} else {
				problem->perform_action(get<2>(next));
			}
		}

		double target_val = problem->score_result();
		target_val -= 0.0001 * solution_wrapper->num_actions;

		solution_wrapper->experiment_end(target_val);

		delete problem;

		if (solution_wrapper->solution->timestamp != starting_timestamp) {
			solution_wrapper->save(path, filename);
		}
	}

	solution_wrapper->clean_scopes();
	solution_wrapper->save(path, filename);

	delete problem_type;
	delete solution_wrapper;

	cout << "Done" << endl;
}
