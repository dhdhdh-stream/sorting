// - just use optical flow
//   - and doesn't need to be perfect of course

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "assign_node.h"
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

	// int starting_x = world.curr_x;
	// int starting_y = world.curr_y;

	vector<int> obs;
	vector<int> actions;

	obs.push_back(world.get_observation());

	uniform_int_distribution<int> action_distribution(0, 3);
	// for (int a_index = 0; a_index < 30; a_index++) {
	for (int a_index = 0; a_index < 60; a_index++) {
		int action = action_distribution(generator);
		world.perform_action(action);
		obs.push_back(world.get_observation());
		actions.push_back(action);
	}

	int num_tries = 0;
	while (true) {
		num_tries++;

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

		Mapping mapping;
		calc_mapping_helper(obs,
							curr_assigment,
							mapping);

		bool is_done = false;
		// for (int i_index = 0; i_index < 100; i_index++) {
		for (int i_index = 0; i_index < 200; i_index++) {
			vector<vector<int>> instance;
			simple_assign(mapping,
						  instance);
			// instance = world.world;

			bool is_valid = check_valid(obs,
										actions,
										instance);

			if (is_valid) {
				cout << "num_tries: " << num_tries << endl;

				cout << "world:" << endl;
				world.print();

				cout << "instance:" << endl;
				for (int x_index = 0; x_index < WORLD_WIDTH; x_index++) {
					for (int y_index = 0; y_index < WORLD_HEIGHT; y_index++) {
						cout << instance[x_index][y_index] << " ";
					}
					cout << endl;
				}

				is_done = true;
				break;
			}
		}

		if (is_done) {
			break;
		}
	}

	// vector<vector<int>> map_vals(WORLD_WIDTH, vector<int>(WORLD_HEIGHT, 0));
	// vector<vector<bool>> map_assigned(WORLD_WIDTH, vector<bool>(WORLD_HEIGHT, false));
	// AssignNode* node = new AssignNode();
	// node->solve(obs,
	// 			actions,
	// 			0,
	// 			starting_x,
	// 			starting_y,
	// 			map_vals,
	// 			map_assigned);

	// world.print();

	cout << "Done" << endl;
}
