/**
 * - i[3]/i[5] + i[4]/i[7] = i[6]/i[8]
 * - i[11]/i[12] + i[13]/i[14] = i[16]
 */

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "all_to_all_network.h"

using namespace std;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	AllToAllNetwork* network = new AllToAllNetwork();

	uniform_int_distribution<int> is_output_distribution(0, 4);

	vector<double> sum_errors(20, 0.0);
	double examine_error = 0.0;
	int examine_count = 0;
	for (int iter_index = 0; iter_index < 1000000; iter_index++) {
		vector<double> vals(20);
		for (int i = 0; i < 20; i++) {
			vals[i] = rand()%11 - 5;
		}

		vals[5] = vals[3];
		vals[7] = vals[4];
		vals[6] = vals[3] + vals[4];
		vals[8] = vals[6];

		vals[12] = vals[11];
		vals[14] = vals[13];
		vals[16] = vals[11] + vals[13];

		vector<bool> is_output(20);
		for (int i = 0; i < 20; i++) {
			is_output[i] = is_output_distribution(generator) == 0;
		}

		vector<double> inputs(20);
		for (int i = 0; i < 20; i++) {
			if (is_output[i]) {
				inputs[i] = 0.0;
			} else {
				inputs[i] = vals[i];
			}
		}

		network->activate(inputs,
						  is_output);

		vector<double> errors(20);
		for (int i = 0; i < 20; i++) {
			if (is_output[i]) {
				sum_errors[i] += abs(vals[i] - network->output->acti_vals[i]);

				errors[i] = vals[i] - network->output->acti_vals[i];
			} else {
				errors[i] = 0.0;

				// errors[i] = vals[i] - network->output->acti_vals[i];
			}
		}

		if (is_output[16]
				&& !is_output[11]
				&& !is_output[13]) {
			examine_error += abs(vals[16] - network->output->acti_vals[16]);
			examine_count++;
		}

		network->backprop(errors,
						  is_output);

		if (iter_index%10000 == 0) {
			cout << "sum_errors:" << endl;
			for (int i = 0; i < 20; i++) {
				cout << i << ": " << sum_errors[i] << endl;
			}

			cout << "examine_error: " << examine_error << endl;
			cout << "examine_count: " << examine_count << endl;

			cout << endl;

			for (int i = 0; i < 20; i++) {
				sum_errors[i] = 0.0;
			}

			examine_error = 0.0;
			examine_count = 0;
		}
	}

	double sum_true_variance = 0.0;
	for (int iter_index = 0; iter_index < 1000; iter_index++) {
		double val = rand()%11 - 5;
		sum_true_variance += val * val;
	}
	double true_variance = sqrt(sum_true_variance / 1000);
	cout << "true_variance: " << true_variance << endl;

	double sum_bad_variance = 0.0;
	for (int iter_index = 0; iter_index < 1000; iter_index++) {
		vector<double> vals(20);
		for (int i = 0; i < 20; i++) {
			vals[i] = rand()%11 - 5;
		}

		vals[5] = vals[3];
		vals[7] = vals[4];
		vals[6] = vals[3] + vals[4];
		vals[8] = vals[6];

		vals[12] = vals[11];
		vals[14] = vals[13];
		vals[16] = vals[11] + vals[13];

		vector<bool> is_output(20, false);
		is_output[11] = true;
		is_output[16] = true;

		vector<double> inputs(20);
		for (int i = 0; i < 20; i++) {
			if (is_output[i]) {
				inputs[i] = 0.0;
			} else {
				inputs[i] = vals[i];
			}
		}

		network->activate(inputs,
						  is_output);

		sum_bad_variance += (vals[16] - network->output->acti_vals[16]) * (vals[16] - network->output->acti_vals[16]);
	}
	double bad_variance = sqrt(sum_bad_variance / 1000);
	cout << "bad_variance: " << bad_variance << endl;

	double sum_good_variance = 0.0;
	for (int iter_index = 0; iter_index < 1000; iter_index++) {
		vector<double> vals(20);
		for (int i = 0; i < 20; i++) {
			vals[i] = rand()%11 - 5;
		}

		vals[5] = vals[3];
		vals[7] = vals[4];
		vals[6] = vals[3] + vals[4];
		vals[8] = vals[6];

		vals[12] = vals[11];
		vals[14] = vals[13];
		vals[16] = vals[11] + vals[13];

		vector<bool> is_output(20, false);
		is_output[16] = true;

		vector<double> inputs(20);
		for (int i = 0; i < 20; i++) {
			if (is_output[i]) {
				inputs[i] = 0.0;
			} else {
				inputs[i] = vals[i];
			}
		}

		network->activate(inputs,
						  is_output);

		sum_good_variance += (vals[16] - network->output->acti_vals[16]) * (vals[16] - network->output->acti_vals[16]);
	}
	double good_variance = sqrt(sum_good_variance / 1000);
	cout << "good_variance: " << good_variance << endl;

	delete network;

	cout << "Done" << endl;
}
