#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "keypoint.h"
#include "keypoint_helpers.h"
#include "world_model.h"
#include "world_model_helpers.h"
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

	WorldModel* world_model = new WorldModel();

	while (world_model->keypoints.size() < 10) {
		Keypoint* potential = create_potential_keypoint(world_truth);

		if (potential_keypoint_is_duplicate(world_model, potential)) {
			delete potential;
			continue;
		}

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
				int new_keypoint_index = world_model->keypoints.size();
				world_model->keypoints.push_back(potential);
				for (int p_index = 0; p_index < (int)path_actions.size(); p_index++) {
					world_model->path_actions.push_back(path_actions[p_index]);
					world_model->path_obs.push_back(path_obs[p_index]);
					world_model->path_start_indexes.push_back(new_keypoint_index);
					world_model->path_end_indexes.push_back(new_keypoint_index);
					world_model->path_success_likelihoods.push_back(0.0);
				}

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

				cout << "paths:" << endl;
				for (int p_index = 0; p_index < (int)path_actions.size(); p_index++) {
					cout << p_index << ":";
					for (int a_index = 0; a_index < (int)path_actions[p_index].size(); a_index++) {
						cout << " " << path_actions[p_index][a_index];
					}
					cout << endl;
				}
			} else {
				delete potential;
			}
		} else {
			delete potential;
		}
	}

	ofstream output_file;
	output_file.open("save.txt");
	world_truth->save(output_file);
	world_model->save(output_file);
	output_file.close();

	delete world_model;
	delete world_truth;

	cout << "Done" << endl;
}
