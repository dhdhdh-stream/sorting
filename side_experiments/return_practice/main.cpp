// TODO: handle infinite continuation

// - branch node uses state
//   - state values constantly changing, but branch node decision making won't
//     - so how branch node behaves will change...
//       - ...but changes will be taken into account by predict
//         - so shouldn't be too damaging?

// - to create secondary factors, use error gradient as target val

// - when creating a branch/experiment:
//   - create entirely using existing samples
//   - once created, essentially commit solution to it
//   - if actually bad, hope that another change fixes it
//     - or can remove, but save samples

// TODO: dual option backprop
// - then try changing inputs

// TODO: retrain branch networks on train as well

// TODO: impossible to negate own value
// - maybe add forget?
// - or simply allow referring self

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "constants.h"
#include "run.h"
#include "simple.h"
#include "world_model_helpers.h"
#include "world_model_wrapper.h"

using namespace std;

int seed;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	ProblemType* problem_type = new TypeSimple();

	string filename;
	WorldModelWrapper* wrapper;
	if (argc > 1) {
		filename = argv[1];
		wrapper = new WorldModelWrapper(
			"saves/",
			filename);
	} else {
		filename = "main.txt";
		wrapper = new WorldModelWrapper(problem_type);
	}

	int iter_index = 0;
	while (true) {
		Problem* problem = problem_type->get_problem();

		Run run;
		init_run(run,
				 wrapper);

		while (true) {
			vector<double> obs = problem->get_observations();
			explore_obs_step(obs,
							 run,
							 wrapper);

			int next_action;
			bool is_done = false;
			explore_action_step(run,
								next_action,
								is_done,
								wrapper);

			if (is_done) {
				break;
			} else {
				problem->perform_action(next_action);
			}
		}

		double target_val = problem->score_result();
		// temp
		cout << "target_val: " << target_val << endl;

		wrapper->sample_obs.push_back(run.obs_histories);
		wrapper->sample_actions.push_back(run.action_histories);
		wrapper->sample_target_vals.push_back(target_val);

		delete problem;

		iter_index++;
		if (iter_index % SAMPLES_PER_TRAIN == 0) {
			train_helper(wrapper);

			wrapper->save("saves/", filename);
		}
	}

	delete problem_type;
	delete wrapper;

	cout << "Done" << endl;
}
