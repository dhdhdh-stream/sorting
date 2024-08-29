#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

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

	cout << "world_model->keypoints[0]->obs:";
	for (int o_index = 0; o_index < (int)world_model->keypoints[0]->obs.size(); o_index++) {
		cout << " " << world_model->keypoints[0]->obs[o_index];
	}
	cout << endl;

	cout << "world_model->keypoints[0]->actions:";
	for (int a_index = 0; a_index < (int)world_model->keypoints[0]->actions.size(); a_index++) {
		cout << " " << world_model->keypoints[0]->actions[a_index];
	}
	cout << endl;

	delete world_model;
	delete world_truth;

	cout << "Done" << endl;
}
