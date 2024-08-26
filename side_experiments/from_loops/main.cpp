/**
 * TODO:
 * - don't worry about merging for now?
 *   - just go into unknown and see if can return
 * - but need to merge to know what to try and what not to try?
 *   - otherwise, can travel into "unknown" from a duplicate spot
 *     - create a bunch of extra state that all, along with original, all actually go back to same duplicate
 *   - but merging may not be correct as only specific sequence tested?
 *     - may not work if actions performed in arbitrary order
 * 
 * - perhaps first try cutting
 * 
 * - need to be prepared to randomly enter into the unknown
 * 
 * - also, explore into unknown instead of random
 */

/**
 * - problems that humans are able to solve:
 *   - 2D/3D obviously
 *   - virtual world within a game
 *   - analog clock face
 */

/**
 * - if something unexpected happened, cannot be sure whether:
 *   - random transition or obs
 *   - secretly in a different state
 *     - try to isolate strange situations and see if can identify patterns?
 */

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

const int UNKNOWN_EXPLORE_NUM_ACTIONS = 4;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	WorldTruth* world_truth = new WorldTruth();

	WorldModel* world_model = find_stable(world_truth);

	cout << "world_truth->curr_x: " << world_truth->curr_x << endl;
	cout << "world_truth->curr_y: " << world_truth->curr_y << endl;

	vector<int> unknown_actions;

	uniform_int_distribution<int> action_distribution(0, NUM_ACTIONS-1);
	while (true) {
		cout << "iter" << endl;

		for (int a_index = 0; a_index < UNKNOWN_EXPLORE_NUM_ACTIONS; a_index++) {
			int random_action = action_distribution(generator);
			world_truth->move(random_action);
			unknown_actions.push_back(random_action);
		}

		cout << "world_truth->curr_x: " << world_truth->curr_x << endl;
		cout << "world_truth->curr_y: " << world_truth->curr_y << endl;

		bool is_success = localize(world_truth,
								   world_model,
								   unknown_actions);

		if (is_success) {
			break;
		}
	}

	cout << "unknown_actions:";
	for (int a_index = 0; a_index < (int)unknown_actions.size(); a_index++) {
		cout << " " << unknown_actions[a_index];
	}
	cout << endl;

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
