#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

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

	double starting_misguess = measure_model(curr_model);
	cout << "starting_misguess: " << starting_misguess << endl;

	WorldModel* best_model = curr_model;
	double best_misguess = starting_misguess;
	for (int iter_index = 0; iter_index < 20; iter_index++) {
		WorldModel* next_model = new WorldModel(curr_model);
		next_model->split_state(6);

		train_model(next_model);

		double next_misguess = measure_model(next_model);
		cout << "next_misguess: " << next_misguess << endl;

		if (next_misguess < best_misguess) {
			if (best_model != curr_model) {
				delete best_model;
			}
			best_model = next_model;
			best_misguess = next_misguess;
		} else {
			delete next_model;
		}
	}

	{
		ofstream output_file;
		output_file.open("split.txt");
		best_model->save(output_file);
		output_file.close();
	}

	delete curr_model;

	cout << "Done" << endl;
}
