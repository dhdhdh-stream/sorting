#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "constants.h"
#include "keypoint.h"
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

	WorldModel* world_model = new WorldModel();

	ifstream input_file;
	input_file.open("save.txt");
	world_truth->load(input_file);
	world_model->load(input_file);
	input_file.close();

	cout << "world_truth->world_size: " << world_truth->world_size << endl;
	cout << "world_truth->obj_x_vel: " << world_truth->obj_x_vel << endl;
	cout << "world_truth->obj_y_vel: " << world_truth->obj_y_vel << endl;

	uniform_int_distribution<int> action_distribution(0, NUM_ACTIONS-1);
	uniform_int_distribution<int> unknown_distribution(0, 9);
	for (int k_index = 0; k_index < (int)world_model->keypoints.size(); k_index++) {
		cout << "world_model->keypoints[k_index]->obs:";
		for (int o_index = 0; o_index < (int)world_model->keypoints[k_index]->obs.size(); o_index++) {
			cout << " " << world_model->keypoints[k_index]->obs[o_index];
		}
		cout << endl;

		cout << "world_model->keypoints[k_index]->actions:";
		for (int a_index = 0; a_index < (int)world_model->keypoints[k_index]->actions.size(); a_index++) {
			cout << " " << world_model->keypoints[k_index]->actions[a_index];
		}
		cout << endl;

		for (int i_index = 0; i_index < 5; i_index++) {
			while (true) {
				int num_initial_unknown = unknown_distribution(generator);
				for (int a_index = 0; a_index < num_initial_unknown; a_index++) {
					world_truth->move(action_distribution(generator));
				}

				bool is_match = world_model->keypoints[k_index]->match(world_truth);

				if (is_match) {
					break;
				}
			}

			cout << "world_truth->curr_x: " << world_truth->curr_x << endl;
			cout << "world_truth->curr_y: " << world_truth->curr_y << endl;
			cout << "world_truth->curr_direction: " << world_truth->curr_direction << endl;
			cout << "world_truth->action_queue:";
			for (int a_index = 0; a_index < (int)world_truth->action_queue.size(); a_index++) {
				cout << " " << world_truth->action_queue[a_index];
			}
			cout << endl;
			cout << endl;
		}

		cout << endl;
	}

	delete world_model;
	delete world_truth;

	cout << "Done" << endl;
}
