#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "network.h"

using namespace std;

int seed;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	Network* noise_network_1 = new Network(1);
	Network* noise_network_2 = new Network(1);
	Network* noise_network_3 = new Network(1);
	Network* noise_network_4 = new Network(1);

	Network* error_network_1 = new Network(1);
	Network* error_network_2 = new Network(1);
	Network* error_network_3 = new Network(1);
	Network* error_network_4 = new Network(1);

	uniform_int_distribution<int> base_distribution(0, 1);
	normal_distribution<double> noise_distribution(0, 1);
	uniform_int_distribution<int> error_direction_distribution(0, 1);
	for (int iter_index = 0; iter_index < 10000000; iter_index++) {
		double base = -0.5 + base_distribution(generator);

		double noise_1 = 0.1 * noise_distribution(generator);
		double base_1 = base + noise_1;

		double noise_2 = 0.2 * noise_distribution(generator);
		// double noise_2 = 0.25 * noise_distribution(generator);
		double base_2 = base_1 + noise_2;

		double noise_3 = 0.5 * noise_distribution(generator);
		// double noise_3 = 0.8 * noise_distribution(generator);
		double base_3 = base_2 + noise_3;

		double noise_4 = noise_distribution(generator);
		// double noise_4 = 2.0 * noise_distribution(generator);
		double base_4 = base_3 + noise_4;

		vector<double> inputs_4{base_4};
		noise_network_4->activate(inputs_4);
		double predicted_noise_4 = noise_network_4->output->acti_vals[0];
		noise_network_4->backprop(noise_4 - predicted_noise_4);
		double error_4 = abs(noise_4 - predicted_noise_4);
		error_network_4->activate(inputs_4);
		double predicted_error_4 = error_network_4->output->acti_vals[0];
		error_network_4->backprop(error_4 - predicted_error_4);

		double predicted_base_3 = base_4 - predicted_noise_4;
		if (error_direction_distribution(generator) == 0) {
			predicted_base_3 += predicted_error_4;
		} else {
			predicted_base_3 -= predicted_error_4;
		}

		{
			vector<double> inputs_3{base_3};
			noise_network_3->activate(inputs_3);
			double predicted_noise_3 = noise_network_3->output->acti_vals[0];
			noise_network_3->backprop(noise_3 - predicted_noise_3);
			double error_3 = abs(noise_3 - predicted_noise_3);
			error_network_3->activate(inputs_3);
			error_network_3->backprop(error_3 - error_network_3->output->acti_vals[0]);
		}

		vector<double> inputs_3{predicted_base_3};
		noise_network_3->activate(inputs_3);
		double predicted_noise_3 = noise_network_3->output->acti_vals[0];
		error_network_3->activate(inputs_3);
		double predicted_error_3 = error_network_3->output->acti_vals[0];

		double predicted_base_2 = predicted_base_3 - predicted_noise_3;
		if (error_direction_distribution(generator) == 0) {
			predicted_base_2 += predicted_error_3;
		} else {
			predicted_base_2 -= predicted_error_3;
		}

		{
			vector<double> inputs_2{base_2};
			noise_network_2->activate(inputs_2);
			double predicted_noise_2 = noise_network_2->output->acti_vals[0];
			noise_network_2->backprop(noise_2 - predicted_noise_2);
			double error_2 = abs(noise_2 - predicted_noise_2);
			error_network_2->activate(inputs_2);
			error_network_2->backprop(error_2 - error_network_2->output->acti_vals[0]);
		}

		vector<double> inputs_2{predicted_base_2};
		noise_network_2->activate(inputs_2);
		double predicted_noise_2 = noise_network_2->output->acti_vals[0];
		error_network_2->activate(inputs_2);
		double predicted_error_2 = error_network_2->output->acti_vals[0];

		double predicted_base_1 = predicted_base_2 - predicted_noise_2;
		if (error_direction_distribution(generator) == 0) {
			predicted_base_1 += predicted_error_2;
		} else {
			predicted_base_1 -= predicted_error_2;;
		}

		{
			vector<double> inputs_1{base_1};
			noise_network_1->activate(inputs_1);
			double predicted_noise_1 = noise_network_1->output->acti_vals[0];
			noise_network_1->backprop(noise_1 - predicted_noise_1);
			double error_1 = abs(noise_1 - predicted_noise_1);
			error_network_1->activate(inputs_1);
			error_network_1->backprop(error_1 - error_network_1->output->acti_vals[0]);
		}

		vector<double> inputs_1{predicted_base_1};
		noise_network_1->activate(inputs_1);
		double predicted_noise_1 = noise_network_1->output->acti_vals[0];
		error_network_1->activate(inputs_1);
		double predicted_error_1 = error_network_1->output->acti_vals[0];

		double predicted_base = predicted_base_1 - predicted_noise_1;
		if (error_direction_distribution(generator) == 0) {
			predicted_base += predicted_error_1;
		} else {
			predicted_base -= predicted_error_1;
		}

		if ((iter_index + 1) % 100000 == 0) {
			cout << iter_index << endl;
			cout << "base: " << base << endl;
			cout << "base_4: " << base_4 << endl;
			cout << "predicted_base: " << predicted_base << endl;
			cout << endl;
		}
	}

	cout << "Done" << endl;
}
