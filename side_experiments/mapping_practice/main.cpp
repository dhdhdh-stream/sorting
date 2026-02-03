// - can't make an adjustment and the appropriate pair adjustment at the same time

// - should just split into assignment and Markov
// - to assign, random run, then fix obs+spots in random order

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "mapping_helpers.h"
#include "world.h"

using namespace std;

int seed;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	World world;

	vector<int> obs;
	vector<int> actions;

	obs.push_back(world.get_observation());

	uniform_int_distribution<int> action_distribution(0, 3);
	// for (int a_index = 0; a_index < 30; a_index++) {
	for (int a_index = 0; a_index < 80; a_index++) {
		int action = action_distribution(generator);
		world.perform_action(action);
		obs.push_back(world.get_observation());
		actions.push_back(action);
	}

	Assignment best_assignment;
	double best_conflict = numeric_limits<double>::max();
	for (int try_index = 0; try_index < 100; try_index++) {
		cout << "try_index: " << try_index << endl;

		Assignment curr_assigment;
		init_assignment_helper(actions,
							   curr_assigment);

		double curr_conflict;
		{
			Mapping mapping;
			calc_mapping_helper(obs,
								curr_assigment,
								mapping);

			curr_conflict = calc_conflict_helper(mapping);
		}

		// for (int step_index = 0; step_index < 100; step_index++) {
		for (int step_index = 0; step_index < 200; step_index++) {
			Assignment best_mod = curr_assigment;
			double best_mod_conflict = curr_conflict;
			// for (int modify_index = 0; modify_index < 100; modify_index++) {
			for (int modify_index = 0; modify_index < 200; modify_index++) {
				Assignment potential_mod = curr_assigment;
				modify_assignment_helper(actions,
										 potential_mod);

				double potential_mod_conflict;
				{
					Mapping mapping;
					calc_mapping_helper(obs,
										potential_mod,
										mapping);

					potential_mod_conflict = calc_conflict_helper(mapping);
				}

				if (potential_mod_conflict < curr_conflict) {
					best_mod = potential_mod;
					best_mod_conflict = potential_mod_conflict;
				}
			}

			curr_assigment = best_mod;
			curr_conflict = best_mod_conflict;
		}

		cout << "curr_conflict: " << curr_conflict << endl;

		if (curr_conflict < best_conflict) {
			best_assignment = curr_assigment;
			best_conflict = curr_conflict;
		}
	}

	cout << "best_conflict: " << best_conflict << endl;

	world.print();
	{
		Mapping mapping;
		calc_mapping_helper(obs,
							best_assignment,
							mapping);

		mapping.print();
	}

	cout << "Done" << endl;
}
