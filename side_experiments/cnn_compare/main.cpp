#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "cnn.h"
#include "minesweeper.h"

using namespace std;

int seed;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	CNN cnn;

	double sum_error = 0.0;
	for (int iter_index = 0; iter_index < 300000; iter_index++) {
		Minesweeper minesweeper;

		vector<vector<double>> input(11);
		for (int x_index = 0; x_index < 11; x_index++) {
			input[x_index] = vector<double>(11);
		}
		for (int x_index = 0; x_index < 11; x_index++) {
			for (int y_index = 0; y_index < 11; y_index++) {
				input[x_index][y_index] = minesweeper.get_observation_helper(x_index-1, y_index-1);
			}
		}

		cnn.activate(input);

		double target_val = minesweeper.score_result();

		double predicted = cnn.output->acti_vals[0];

		sum_error += abs(target_val - predicted);

		double error = target_val - predicted;
		cnn.backprop(error);

		if (iter_index%10000 == 0) {
			cout << "sum_error: " << sum_error << endl;
			sum_error = 0.0;
		}
	}

	cout << "Done" << endl;
}
