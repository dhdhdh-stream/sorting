#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "constants.h"
#include "find_stable_helpers.h"
#include "localize_helpers.h"
#include "world_model.h"
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

	WorldTruth* world_truth = new WorldTruth();

	WorldModel* world_model = find_stable(world_truth);

	uniform_int_distribution<int> action_distribution(0, NUM_ACTIONS-1);
	for (int a_index = 0; a_index < 10; a_index++) {
		int random_action = action_distribution(generator);
		world_truth->move(random_action);
	}

	localize(world_truth,
			 world_model);

	{
		ofstream output_file;
		output_file.open("display.txt");
		world_model->save_for_display(output_file);
		output_file.close();
	}

	delete world_model;
	delete world_truth;

	cout << "Done" << endl;
}
