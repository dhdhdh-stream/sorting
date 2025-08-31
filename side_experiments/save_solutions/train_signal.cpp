// TODO: iterative match
// - first split, then fully train score
//   - then train match according to whether helps

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "globals.h"
#include "helpers.h"
#include "signal_experiment.h"
#include "simpler.h"
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

	for (int iter_index = 0; iter_index < 10; iter_index++) {
		solution_wrapper->signal_experiment = new SignalExperiment(
			0,
			solution_wrapper);

		while (true) {
			Problem* problem = problem_type->get_problem();
			solution_wrapper->problem = problem;

			solution_wrapper->signal_experiment_init();

			while (true) {
				vector<double> obs = problem->get_observations();

				tuple<bool,bool,int> next = solution_wrapper->signal_experiment_step(obs);
				if (get<0>(next)) {
					break;
				} else if (get<1>(next)) {
					uniform_int_distribution<int> action_distribution(0, problem_type->num_possible_actions()-1);
					int new_action = action_distribution(generator);

					solution_wrapper->signal_experiment_set_action(new_action);

					problem->perform_action(new_action);
				} else {
					problem->perform_action(get<2>(next));
				}
			}

			double target_val = problem->score_result();
			target_val -= 0.0001 * solution_wrapper->num_actions;

			solution_wrapper->signal_experiment_end(target_val);

			delete problem;

			if (solution_wrapper->signal_experiment->state == SIGNAL_EXPERIMENT_STATE_DONE) {
				break;
			}
		}

		delete solution_wrapper->signal_experiment;
	}

	for (int s_index = 0; s_index < (int)solution_wrapper->solutions.size(); s_index++) {
		cout << "s_index: " << s_index << endl;

		for (int i_index = 0; i_index < 4; i_index++) {
			Problem* problem = problem_type->get_problem();

			solution_wrapper->init(s_index);

			while (true) {
				vector<double> obs = problem->get_observations();

				pair<bool,int> next = solution_wrapper->step(obs);
				if (next.first) {
					break;
				} else {
					problem->perform_action(next.second);
				}
			}

			cout << "i_index: " << i_index << endl;
			problem->print();
			double signal = calc_signal(solution_wrapper->scope_histories[0],
										solution_wrapper);
			cout << "signal: " << signal << endl;

			solution_wrapper->end();

			delete problem;
		}
	}

	// solution_wrapper->save("saves/", filename);

	delete problem_type;
	delete solution_wrapper;

	cout << "Done" << endl;
}
