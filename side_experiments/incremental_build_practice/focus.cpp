/**
 * TODO:
 * - if 90% go up, then doesn't actually want to travel there because can go to corner, go up and miss
 * 
 * - maybe because there is uncertainty for the 1s, that carries over into the 0s
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

	// ifstream input_file;
	// input_file.open("save.txt");
	// WorldModel* curr_model = new WorldModel(input_file);
	// input_file.close();

	// double starting_misguess = measure_model(curr_model);
	// cout << "starting_misguess: " << starting_misguess << endl;

	// WorldModel* best_model = curr_model;
	// double best_misguess = starting_misguess;
	// for (int iter_index = 0; iter_index < 10; iter_index++) {
	// 	WorldModel* next_model = new WorldModel(curr_model);
	// 	next_model->split_state(6);

	// 	train_model(next_model);

	// 	double next_misguess = measure_model(next_model);
	// 	cout << "next_misguess: " << next_misguess << endl;

	// 	if (next_misguess < best_misguess) {
	// 		if (best_model != curr_model) {
	// 			delete best_model;
	// 		}
	// 		best_model = next_model;
	// 		best_misguess = next_misguess;
	// 	} else {
	// 		delete next_model;
	// 	}
	// }

	// {
	// 	ofstream output_file;
	// 	output_file.open("split.txt");
	// 	best_model->save(output_file);
	// 	output_file.close();
	// }

	ifstream input_file;
	input_file.open("snapshot_t2.txt");
	WorldModel* curr_model = new WorldModel(input_file);
	input_file.close();

	// vector<int> new_state_indexes;
	// vector<int> new_actions{ACTION_LEFT};
	// curr_model->add_path(7,
	// 					 4,
	// 					 new_state_indexes,
	// 					 1,
	// 					 ACTION_DOWN,
	// 					 new_actions,
	// 					 ACTION_UP);

	// train_model(curr_model);

	// vector<int> new_state_indexes;
	// vector<int> new_actions{ACTION_RIGHT, ACTION_UP};
	// curr_model->add_path(7,
	// 					 4,
	// 					 new_state_indexes,
	// 					 4,
	// 					 ACTION_DOWN,
	// 					 new_actions,
	// 					 ACTION_LEFT);

	// train_model(curr_model);

	{
		vector<int> new_state_indexes;
		vector<int> new_actions;
		curr_model->add_path(9,
							 4,
							 new_state_indexes,
							 4,
							 ACTION_DOWN,
							 new_actions,
							 ACTION_UP);

		train_model(curr_model);
	}

	{
		vector<int> new_state_indexes;
		vector<int> new_actions;
		curr_model->add_path(9,
							 2,
							 new_state_indexes,
							 2,
							 ACTION_DOWN,
							 new_actions,
							 ACTION_UP);

		train_model(curr_model);
	}

	double curr_misguess = measure_model(curr_model);
	cout << "curr_misguess: " << curr_misguess << endl;

	{
		ofstream output_file;
		output_file.open("display.txt");
		curr_model->save_for_display(output_file);
		output_file.close();
	}

	// {
	// 	ofstream output_file;
	// 	output_file.open("split.txt");
	// 	curr_model->save(output_file);
	// 	output_file.close();
	// }

	delete curr_model;

	cout << "Done" << endl;
}
