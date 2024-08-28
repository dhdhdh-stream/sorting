#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "key_point.h"
#include "key_point_helpers.h"
#include "world_truth.h"

using namespace std;

int seed;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	// seed = (unsigned)time(NULL);
	seed = 1724809896;
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	WorldTruth* world_truth = new WorldTruth();

	cout << "world_truth->world_size: " << world_truth->world_size << endl;
	cout << "world_truth->obj_x_vel: " << world_truth->obj_x_vel << endl;
	cout << "world_truth->obj_y_vel: " << world_truth->obj_y_vel << endl;

	KeyPoint* key_point;
	while (true) {
		KeyPoint* potential = create_potential_key_point(world_truth);

		cout << "potential->obs:";
		for (int o_index = 0; o_index < (int)potential->obs.size(); o_index++) {
			cout << " " << potential->obs[o_index];
		}
		cout << endl;

		cout << "potential->actions:";
		for (int a_index = 0; a_index < (int)potential->actions.size(); a_index++) {
			cout << " " << potential->actions[a_index];
		}
		cout << endl;

		vector<vector<double>> path_obs;
		vector<vector<int>> path_actions;
		bool find_paths_success = find_paths_for_potential(
			world_truth,
			potential,
			path_obs,
			path_actions);

		if (find_paths_success) {
			bool verify_unique_success = verify_potential_uniqueness(
				world_truth,
				potential,
				path_actions);

			if (verify_unique_success) {
				key_point = potential;

				cout << "key_point->obs:";
				for (int o_index = 0; o_index < (int)key_point->obs.size(); o_index++) {
					cout << " " << key_point->obs[o_index];
				}
				cout << endl;

				cout << "key_point->actions:";
				for (int a_index = 0; a_index < (int)key_point->actions.size(); a_index++) {
					cout << " " << key_point->actions[a_index];
				}
				cout << endl;

				cout << "paths:" << endl;
				for (int p_index = 0; p_index < (int)path_actions.size(); p_index++) {
					cout << p_index << ":";
					for (int a_index = 0; a_index < (int)path_actions[p_index].size(); a_index++) {
						cout << " " << path_actions[p_index][a_index];
					}
					cout << endl;
				}

				break;
			} else {
				delete potential;
			}
		} else {
			delete potential;
		}
	}

	delete key_point;
	delete world_truth;

	cout << "Done" << endl;
}
