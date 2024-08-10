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

	cout << "starting_likelihood:" << endl;
	for (int s_index = 0; s_index < (int)curr_model->starting_likelihood.size(); s_index++) {
		cout << s_index << ": " << curr_model->starting_likelihood[s_index] << endl;
	}

	delete curr_model;

	cout << "Done" << endl;
}
