// TODO: test with input
// TODO: test with variance

// 2D: 51 -> 34 -> 23
// 3D: 75 -> 59 -> 48

#include <algorithm>
#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "constants.h"
#include "network.h"

using namespace std;

int seed;

default_random_engine generator;

// const int NUM_DIMENSIONS = 2;
const int NUM_DIMENSIONS = 3;

const int MEASURE_ITERS = 4000;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	vector<Network*> networks;
	networks.push_back(new Network(0, NUM_DIMENSIONS));

	normal_distribution<double> target_distribution(0.0, 5.0);

	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		vector<vector<double>> predicted(networks.size());
		vector<double> inputs;
		for (int n_index = 0; n_index < (int)networks.size(); n_index++) {
			networks[n_index]->activate(inputs);
			vector<double> curr_predicted(NUM_DIMENSIONS);
			for (int d_index = 0; d_index < NUM_DIMENSIONS; d_index++) {
				curr_predicted[d_index] = networks[n_index]->output->acti_vals[d_index];
			}
			predicted[n_index] = curr_predicted;
		}

		vector<double> target_val(NUM_DIMENSIONS);
		for (int d_index = 0; d_index < NUM_DIMENSIONS; d_index++) {
			target_val[d_index] = target_distribution(generator);
		}

		int min_index;
		double min_error = numeric_limits<double>::max();
		for (int n_index = 0; n_index < (int)networks.size(); n_index++) {
			double curr_error = 0.0;
			for (int d_index = 0; d_index < NUM_DIMENSIONS; d_index++) {
				curr_error += (target_val[d_index] - predicted[n_index][d_index])
					* (target_val[d_index] - predicted[n_index][d_index]);
			}
			if (curr_error < min_error) {
				min_error = curr_error;
				min_index = n_index;
			}
		}

		vector<double> errors(NUM_DIMENSIONS);
		for (int d_index = 0; d_index < NUM_DIMENSIONS; d_index++) {
			errors[d_index] = target_val[d_index] - networks[min_index]->output->acti_vals[d_index];
		}
		networks[min_index]->backprop(errors);

		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
			cout << "min_index: " << min_index << endl;
			for (int n_index = 0; n_index < (int)networks.size(); n_index++) {
				for (int d_index = 0; d_index < NUM_DIMENSIONS; d_index++) {
					cout << n_index << " " << d_index << ": " << predicted[n_index][d_index] << endl;
				}
			}
			cout << endl;
		}
	}

	{
		vector<double> misguesses(MEASURE_ITERS);
		for (int iter_index = 0; iter_index < MEASURE_ITERS; iter_index++) {
			vector<double> target_val(NUM_DIMENSIONS);
			for (int d_index = 0; d_index < NUM_DIMENSIONS; d_index++) {
				target_val[d_index] = target_distribution(generator);
			}

			double min_error = numeric_limits<double>::max();
			vector<double> inputs;
			for (int n_index = 0; n_index < (int)networks.size(); n_index++) {
				networks[n_index]->activate(inputs);
				double curr_error = 0.0;
				for (int d_index = 0; d_index < NUM_DIMENSIONS; d_index++) {
					curr_error += (target_val[d_index] - networks[n_index]->output->acti_vals[d_index])
						* (target_val[d_index] - networks[n_index]->output->acti_vals[d_index]);
				}
				if (curr_error < min_error) {
					min_error = curr_error;
				}
			}

			misguesses[iter_index] = min_error;
		}

		double sum_misguess = 0.0;
		for (int iter_index = 0; iter_index < MEASURE_ITERS; iter_index++) {
			sum_misguess += misguesses[iter_index];
		}
		double misguess_average = sum_misguess / (double)MEASURE_ITERS;
		cout << "misguess_average: " << misguess_average << endl;

		double sum_variances = 0.0;
		for (int iter_index = 0; iter_index < MEASURE_ITERS; iter_index++) {
			sum_variances += (misguesses[iter_index] - misguess_average)
				* (misguesses[iter_index] - misguess_average);
		}
		double variance_average = sum_variances / (double)MEASURE_ITERS;
		cout << "variance_average: " << variance_average << endl;
	}

	{
		Network* new_network = new Network(networks[0]);
		new_network->twiddle();
		networks.push_back(new_network);
	}

	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		vector<vector<double>> predicted(networks.size());
		vector<double> inputs;
		for (int n_index = 0; n_index < (int)networks.size(); n_index++) {
			networks[n_index]->activate(inputs);
			vector<double> curr_predicted(NUM_DIMENSIONS);
			for (int d_index = 0; d_index < NUM_DIMENSIONS; d_index++) {
				curr_predicted[d_index] = networks[n_index]->output->acti_vals[d_index];
			}
			predicted[n_index] = curr_predicted;
		}

		vector<double> target_val(NUM_DIMENSIONS);
		for (int d_index = 0; d_index < NUM_DIMENSIONS; d_index++) {
			target_val[d_index] = target_distribution(generator);
		}

		int min_index;
		double min_error = numeric_limits<double>::max();
		for (int n_index = 0; n_index < (int)networks.size(); n_index++) {
			double curr_error = 0.0;
			for (int d_index = 0; d_index < NUM_DIMENSIONS; d_index++) {
				curr_error += (target_val[d_index] - predicted[n_index][d_index])
					* (target_val[d_index] - predicted[n_index][d_index]);
			}
			if (curr_error < min_error) {
				min_error = curr_error;
				min_index = n_index;
			}
		}

		vector<double> errors(NUM_DIMENSIONS);
		for (int d_index = 0; d_index < NUM_DIMENSIONS; d_index++) {
			errors[d_index] = target_val[d_index] - networks[min_index]->output->acti_vals[d_index];
		}
		networks[min_index]->backprop(errors);

		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
			cout << "min_index: " << min_index << endl;
			for (int n_index = 0; n_index < (int)networks.size(); n_index++) {
				for (int d_index = 0; d_index < NUM_DIMENSIONS; d_index++) {
					cout << n_index << " " << d_index << ": " << predicted[n_index][d_index] << endl;
				}
			}
			cout << endl;
		}
	}

	{
		vector<double> misguesses(MEASURE_ITERS);
		for (int iter_index = 0; iter_index < MEASURE_ITERS; iter_index++) {
			vector<double> target_val(NUM_DIMENSIONS);
			for (int d_index = 0; d_index < NUM_DIMENSIONS; d_index++) {
				target_val[d_index] = target_distribution(generator);
			}

			double min_error = numeric_limits<double>::max();
			vector<double> inputs;
			for (int n_index = 0; n_index < (int)networks.size(); n_index++) {
				networks[n_index]->activate(inputs);
				double curr_error = 0.0;
				for (int d_index = 0; d_index < NUM_DIMENSIONS; d_index++) {
					curr_error += (target_val[d_index] - networks[n_index]->output->acti_vals[d_index])
						* (target_val[d_index] - networks[n_index]->output->acti_vals[d_index]);
				}
				if (curr_error < min_error) {
					min_error = curr_error;
				}
			}

			misguesses[iter_index] = min_error;
		}

		double sum_misguess = 0.0;
		for (int iter_index = 0; iter_index < MEASURE_ITERS; iter_index++) {
			sum_misguess += misguesses[iter_index];
		}
		double misguess_average = sum_misguess / (double)MEASURE_ITERS;
		cout << "misguess_average: " << misguess_average << endl;

		double sum_variances = 0.0;
		for (int iter_index = 0; iter_index < MEASURE_ITERS; iter_index++) {
			sum_variances += (misguesses[iter_index] - misguess_average)
				* (misguesses[iter_index] - misguess_average);
		}
		double variance_average = sum_variances / (double)MEASURE_ITERS;
		cout << "variance_average: " << variance_average << endl;
	}

	{
		Network* new_network = new Network(networks[0]);
		new_network->twiddle();
		networks.push_back(new_network);
	}

	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		vector<vector<double>> predicted(networks.size());
		vector<double> inputs;
		for (int n_index = 0; n_index < (int)networks.size(); n_index++) {
			networks[n_index]->activate(inputs);
			vector<double> curr_predicted(NUM_DIMENSIONS);
			for (int d_index = 0; d_index < NUM_DIMENSIONS; d_index++) {
				curr_predicted[d_index] = networks[n_index]->output->acti_vals[d_index];
			}
			predicted[n_index] = curr_predicted;
		}

		vector<double> target_val(NUM_DIMENSIONS);
		for (int d_index = 0; d_index < NUM_DIMENSIONS; d_index++) {
			target_val[d_index] = target_distribution(generator);
		}

		int min_index;
		double min_error = numeric_limits<double>::max();
		for (int n_index = 0; n_index < (int)networks.size(); n_index++) {
			double curr_error = 0.0;
			for (int d_index = 0; d_index < NUM_DIMENSIONS; d_index++) {
				curr_error += (target_val[d_index] - predicted[n_index][d_index])
					* (target_val[d_index] - predicted[n_index][d_index]);
			}
			if (curr_error < min_error) {
				min_error = curr_error;
				min_index = n_index;
			}
		}

		vector<double> errors(NUM_DIMENSIONS);
		for (int d_index = 0; d_index < NUM_DIMENSIONS; d_index++) {
			errors[d_index] = target_val[d_index] - networks[min_index]->output->acti_vals[d_index];
		}
		networks[min_index]->backprop(errors);

		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
			cout << "min_index: " << min_index << endl;
			for (int n_index = 0; n_index < (int)networks.size(); n_index++) {
				for (int d_index = 0; d_index < NUM_DIMENSIONS; d_index++) {
					cout << n_index << " " << d_index << ": " << predicted[n_index][d_index] << endl;
				}
			}
			cout << endl;
		}
	}

	{
		vector<double> misguesses(MEASURE_ITERS);
		for (int iter_index = 0; iter_index < MEASURE_ITERS; iter_index++) {
			vector<double> target_val(NUM_DIMENSIONS);
			for (int d_index = 0; d_index < NUM_DIMENSIONS; d_index++) {
				target_val[d_index] = target_distribution(generator);
			}

			double min_error = numeric_limits<double>::max();
			vector<double> inputs;
			for (int n_index = 0; n_index < (int)networks.size(); n_index++) {
				networks[n_index]->activate(inputs);
				double curr_error = 0.0;
				for (int d_index = 0; d_index < NUM_DIMENSIONS; d_index++) {
					curr_error += (target_val[d_index] - networks[n_index]->output->acti_vals[d_index])
						* (target_val[d_index] - networks[n_index]->output->acti_vals[d_index]);
				}
				if (curr_error < min_error) {
					min_error = curr_error;
				}
			}

			misguesses[iter_index] = min_error;
		}

		double sum_misguess = 0.0;
		for (int iter_index = 0; iter_index < MEASURE_ITERS; iter_index++) {
			sum_misguess += misguesses[iter_index];
		}
		double misguess_average = sum_misguess / (double)MEASURE_ITERS;
		cout << "misguess_average: " << misguess_average << endl;

		double sum_variances = 0.0;
		for (int iter_index = 0; iter_index < MEASURE_ITERS; iter_index++) {
			sum_variances += (misguesses[iter_index] - misguess_average)
				* (misguesses[iter_index] - misguess_average);
		}
		double variance_average = sum_variances / (double)MEASURE_ITERS;
		cout << "variance_average: " << variance_average << endl;
	}

	cout << "Done" << endl;
}
