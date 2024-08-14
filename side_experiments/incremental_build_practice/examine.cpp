/**
 * - any uncertainty magnifies
 *   - if one state is more uncertain than it needs to be, it forces states that don't need to handle a situation to handle a situation
 *     - then those other states become uncertain, and etc.
 * 
 * - but if too certain, may make future improvement tricky
 *   - e.g., if need to expand on a state, or if multiple states initially look the same
 * 
 * TODO: maybe states need locality?
 * - so the jumping around, adding to uncertainty, doesn't occur
 */

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "constants.h"
#include "run_helpers.h"
#include "world_model.h"
#include "world_state.h"
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

	ifstream input_file;
	input_file.open("save.txt");
	WorldModel* curr_model = new WorldModel(input_file);
	input_file.close();

	examine_run(curr_model);

	delete curr_model;

	cout << "Done" << endl;
}
