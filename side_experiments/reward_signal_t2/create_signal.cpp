#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "globals.h"
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

	SignalExperiment* signal_experiment = new SignalExperiment();

	while (true) {
		Problem* problem = problem_type->get_problem();

		SignalExperimentHistory* signal_experiment_history = new SignalExperimentHistory();

		signal_experiment->pre_activate(problem,
										signal_experiment_history);

		signal_experiment->post_activate(problem,
										 signal_experiment_history);

		double target_val = problem->score_result();

		signal_experiment->backprop(target_val,
									signal_experiment_history);

		delete signal_experiment_history;

		delete problem;

		if (signal_experiment->state == SIGNAL_EXPERIMENT_STATE_DONE) {
			break;
		}
	}

	SolutionWrapper* solution_wrapper = new SolutionWrapper(
		problem_type->num_obs());

	signal_experiment->add(solution_wrapper);

	solution_wrapper->save("saves/", "main.txt");

	delete signal_experiment;

	delete problem_type;
	delete solution_wrapper;

	cout << "Done" << endl;
}
