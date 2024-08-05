/**
 * TODO: maybe build up from perfect accuracy?
 * - maybe repeat sequences and look for repeat obs?
 *   - but what about when there's uncertainty?
 * 
 * - if able to handle uncertainty, then might miss things that can be more easily found through exact match?
 *   - like, if there's a cycle to the uncertainty?
 */

// maybe have unknown state that can always go to?
// - limits punishment, but also limits gain

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

	WorldModel* curr_model = new WorldModel();
	// ifstream input_file;
	// input_file.open("save.txt");
	// WorldModel* curr_model = new WorldModel(input_file);
	// input_file.close();

	train_model(curr_model);

	double curr_misguess = measure_model(curr_model);
	cout << "curr_misguess: " << curr_misguess << endl;

	for (int iter_index = 0; iter_index < 20; iter_index++) {
		cout << "iter_index: " << iter_index << endl;

		WorldModel* best_model = curr_model;
		double best_misguess = curr_misguess;

		// w/ extra training
		{
			WorldModel* next_model = new WorldModel(curr_model);

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

		for (int s_index = 0; s_index < (int)curr_model->states.size(); s_index++) {
			WorldModel* next_model = new WorldModel(curr_model);
			next_model->split_state(s_index);

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

		if (best_model != curr_model) {
			delete curr_model;
			curr_model = best_model;
			curr_misguess = best_misguess;
		}

		{
			ofstream output_file;
			output_file.open("save.txt");
			curr_model->save(output_file);
			output_file.close();
		}

		{
			ofstream output_file;
			output_file.open("display.txt");
			curr_model->save_for_display(output_file);
			output_file.close();
		}
	}

	delete curr_model;

	cout << "Done" << endl;
}
