#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "shallow_duplicate_network.h"

using namespace std;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	ShallowDuplicateNetwork* network = new ShallowDuplicateNetwork();

	uniform_int_distribution<int> is_off_distribution(0, 4);

	double sum_error = 0.0;
	for (int iter_index = 0; iter_index < 1000000; iter_index++) {
		// vector<double> inputs(50);
		// vector<double> inputs(200);
		vector<double> inputs(800);
		// for (int i = 0; i < 50; i++) {
		// for (int i = 0; i < 200; i++) {
		for (int i = 0; i < 800; i++) {
			inputs[i] = rand()%11 - 5;
		}

		inputs[2] = inputs[0];
		inputs[1] = inputs[0];

		// vector<bool> is_on(50);
		// vector<bool> is_on(200);
		vector<bool> is_on(800);
		// for (int i = 0; i < 50; i++) {
		// for (int i = 0; i < 200; i++) {
		for (int i = 0; i < 800; i++) {
			is_on[i] = !is_off_distribution(generator) == 0;
		}

		network->activate(inputs,
						  is_on);

		sum_error += abs(inputs[0] - network->output->acti_vals[0]);

		double error = inputs[0] - network->output->acti_vals[0];

		network->backprop(error);

		if (iter_index%10000 == 0) {
			cout << "inputs[0]: " << inputs[0] << endl;
			cout << "network->output->acti_vals[0]: " << network->output->acti_vals[0] << endl;

			cout << "sum_error: " << sum_error << endl;

			cout << endl;

			sum_error = 0.0;
		}
	}

	cout << "0 off" << endl;
	for (int iter_index = 0; iter_index < 4; iter_index++) {
		// vector<double> inputs(50);
		// vector<double> inputs(200);
		vector<double> inputs(800);
		// for (int i = 0; i < 50; i++) {
		// for (int i = 0; i < 200; i++) {
		for (int i = 0; i < 800; i++) {
			inputs[i] = rand()%11 - 5;
		}

		inputs[2] = inputs[0];
		inputs[1] = inputs[0];

		// vector<bool> is_on(50, true);
		// vector<bool> is_on(200, true);
		vector<bool> is_on(800, true);
		is_on[0] = false;

		network->activate(inputs,
						  is_on);

		cout << "inputs[0]: " << inputs[0] << endl;
		cout << "network->output->acti_vals[0]: " << network->output->acti_vals[0] << endl;
	}

	cout << "0 1 off" << endl;
	for (int iter_index = 0; iter_index < 4; iter_index++) {
		// vector<double> inputs(50);
		// vector<double> inputs(200);
		vector<double> inputs(800);
		// for (int i = 0; i < 50; i++) {
		// for (int i = 0; i < 200; i++) {
		for (int i = 0; i < 800; i++) {
			inputs[i] = rand()%11 - 5;
		}

		inputs[2] = inputs[0];
		inputs[1] = inputs[0];

		// vector<bool> is_on(50, true);
		// vector<bool> is_on(200, true);
		vector<bool> is_on(800, true);
		is_on[0] = false;
		is_on[1] = false;

		network->activate(inputs,
						  is_on);

		cout << "inputs[0]: " << inputs[0] << endl;
		cout << "network->output->acti_vals[0]: " << network->output->acti_vals[0] << endl;
	}

	cout << "0 1 2 off" << endl;
	for (int iter_index = 0; iter_index < 4; iter_index++) {
		// vector<double> inputs(50);
		// vector<double> inputs(200);
		vector<double> inputs(800);
		// for (int i = 0; i < 50; i++) {
		// for (int i = 0; i < 200; i++) {
		for (int i = 0; i < 800; i++) {
			inputs[i] = rand()%11 - 5;
		}

		inputs[2] = inputs[0];
		inputs[1] = inputs[0];

		// vector<bool> is_on(50, true);
		// vector<bool> is_on(200, true);
		vector<bool> is_on(800, true);
		is_on[0] = false;
		is_on[1] = false;
		is_on[2] = false;

		network->activate(inputs,
						  is_on);

		cout << "inputs[0]: " << inputs[0] << endl;
		cout << "network->output->acti_vals[0]: " << network->output->acti_vals[0] << endl;
	}

	delete network;

	cout << "Done" << endl;
}
