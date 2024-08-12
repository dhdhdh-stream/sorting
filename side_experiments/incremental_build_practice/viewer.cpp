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
	// input_file.open("split.txt");
	// WorldModel* curr_model = new WorldModel(input_file);
	// input_file.close();

	// ifstream input_file;
	// input_file.open("save.txt");
	// WorldModel* curr_model = new WorldModel(input_file);
	// input_file.close();

	ifstream input_file;
	input_file.open("snapshot.txt");
	WorldModel* curr_model = new WorldModel(input_file);
	input_file.close();

	vector<int> new_state_indexes;
	vector<int> new_actions{ACTION_LEFT};
	curr_model->add_path(7,
						 4,
						 new_state_indexes,
						 1,
						 ACTION_DOWN,
						 new_actions,
						 ACTION_UP);

	{
		ofstream output_file;
		output_file.open("display.txt");
		curr_model->save_for_display(output_file);
		output_file.close();
	}

	delete curr_model;

	cout << "Done" << endl;
}
