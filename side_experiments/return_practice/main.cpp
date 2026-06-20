// TODO: with scopes, even if outer has many states, inner may not need many
// - if inner doesn't affect an outer state, than won't need an inner state to update outer

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "constants.h"
#include "experiment_run.h"
#include "globals.h"
// #include "hit_all.h"
#include "minesweeper.h"
#include "solution.h"
#include "solution_helpers.h"
// #include "test_indirect.h"
#include "world_model.h"
#include "world_model_helpers.h"
#include "wrapper.h"

using namespace std;

int seed;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	// ProblemType* problem_type = new TypeTestIndirect();
	ProblemType* problem_type = new TypeMinesweeper();
	// ProblemType* problem_type = new TypeHitAll();

	string filename;
	Wrapper* wrapper;
	if (argc > 1) {
		filename = argv[1];
		wrapper = new Wrapper("saves/",
							  filename);
	} else {
		filename = "main.txt";
		wrapper = new Wrapper(problem_type);

		init_helper(problem_type,
					wrapper);

		wrapper->save("saves/", filename);
	}

	while (true) {
		int starting_num_states = wrapper->world_model->num_states;
		int starting_timestamp = wrapper->solution->timestamp;

		while (true) {
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

		wrapper->save("saves/", filename);

		wrapper->save_for_display("../", "display.txt");
	}

	delete problem_type;
	delete wrapper;

	cout << "Done" << endl;
}
