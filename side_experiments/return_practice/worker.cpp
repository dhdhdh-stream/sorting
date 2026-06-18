#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "constants.h"
#include "experiment_run.h"
#include "globals.h"
#include "minesweeper.h"
#include "solution.h"
#include "solution_helpers.h"
#include "world_model.h"
#include "world_model_helpers.h"
#include "wrapper.h"

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

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	ProblemType* problem_type = new TypeMinesweeper();

	Wrapper* wrapper = new Wrapper(path, filename);

	auto start_time = chrono::high_resolution_clock::now();

	while (true) {
		int starting_num_states = wrapper->world_model->num_states;
		int starting_timestamp = wrapper->solution->timestamp;

		while (true) {
			auto curr_time = chrono::high_resolution_clock::now();
			auto time_diff = chrono::duration_cast<chrono::seconds>(curr_time - start_time);
			if (time_diff.count() >= 20) {
				start_time = curr_time;

				cout << "a" << endl;
			}

			Problem* problem = problem_type->get_problem();

			ExperimentRun* run = new ExperimentRun();

			wrapper->experiment_init(run);

			while (true) {
				vector<double> obs = problem->get_observations();

				pair<bool,int> next = wrapper->experiment_step(obs,
															   run);
				if (next.first) {
					break;
				} else {
					problem->perform_action(next.second);
				}
			}

			double target_val = problem->score_result();

			wrapper->experiment_end(target_val,
									run);

			delete run;

			delete problem;

			if (wrapper->world_model->num_states != starting_num_states
					|| wrapper->solution->timestamp != starting_timestamp) {
				break;
			}
		}

		wrapper->save(path, filename);
	}

	delete problem_type;
	delete wrapper;

	cout << "Done" << endl;
}
