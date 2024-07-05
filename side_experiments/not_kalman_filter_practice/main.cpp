#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "action_helpers.h"
#include "constants.h"
#include "predicted_world.h"
#include "world_truth.h"

using namespace std;

int seed;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	WorldTruth world_truth;
	PredictedWorld predicted_world(world_truth.vals[3][3]);

	uniform_int_distribution<int> action_distribution(0, 3);
	for (int iter_index = 0; iter_index < 10000; iter_index++) {
		int action = action_distribution(generator);

		int new_obs;
		apply_action(world_truth,
					 action,
					 new_obs);

		update_predicted(predicted_world,
						 action,
						 new_obs);
	}

	cout << "world_truth:" << endl;
	world_truth.print();

	cout << "predicted_world:" << endl;
	predicted_world.print();

	cout << "Done" << endl;
}
