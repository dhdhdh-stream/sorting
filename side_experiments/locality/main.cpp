/**
 * - with fixed transitions, commits too hard to local optima?
 *   - maybe always have transition to unknown?
 */

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

const int TRIES_PER_ITER = 20;
const int CHANGES_PER_TRY = 10;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	WorldModel* curr_model = new WorldModel();
	curr_model->init();
	// ifstream input_file;
	// input_file.open("save.txt");
	// WorldModel* curr_model = new WorldModel(input_file);
	// input_file.close();

	for (int iter_index = 0; iter_index < 300; iter_index++) {
		cout << "iter_index: " << iter_index << endl;

		WorldModel* best_model = NULL;
		double best_misguess = numeric_limits<double>::max();

		for (int t_index = 0; t_index < TRIES_PER_ITER; t_index++) {
			WorldModel* next_model = new WorldModel(curr_model);
			for (int c_index = 0; c_index < CHANGES_PER_TRY; c_index++) {
				next_model->random_change();
			}

			train_model(next_model);

			next_model->clean();

			double next_misguess = measure_model(next_model);
			cout << "next_misguess: " << next_misguess << endl;

			/**
			 * TODO: debug NaN
			 */
			if (next_misguess == next_misguess
					&& next_misguess < best_misguess) {
				if (best_model != NULL) {
					delete best_model;
				}
				best_model = next_model;
				best_misguess = next_misguess;
			} else {
				delete next_model;
			}
		}

		delete curr_model;
		curr_model = best_model;

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
