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

	Network* noise_network_1 = new Network(1, 1);
	Network* noise_network_2 = new Network(1, 1);
	Network* noise_network_3 = new Network(1, 1);
	Network* noise_network_4 = new Network(1, 1);

	double in_mean = 0.0;
	double in_variance = 1.0;
	double end_mean = 0.0;
	double end_variance = 1.0;

	uniform_int_distribution<int> base_distribution(0, 1);
	normal_distribution<double> noise_distribution(0, 1);
	uniform_real_distribution<double> direction_distribution(-1.0, 1.0);
	for (int iter_index = 0; iter_index < 1000000; iter_index++) {
		double base = -0.5 + base_distribution(generator);

		in_mean = 0.999*in_mean + 0.001*base;
		double in_curr_variance = (base - in_mean) * (base - in_mean);
		in_variance = 0.999*in_variance + 0.001*in_curr_variance;

		double in_standard_deviation = sqrt(in_variance);

		double noise_1 = 0.1 * in_standard_deviation * noise_distribution(generator);
		double base_1 = base + noise_1;

		double noise_2 = 0.3 * in_standard_deviation * noise_distribution(generator);
		double base_2 = base_1 + noise_2;

		double noise_3 = 0.8 * in_standard_deviation * noise_distribution(generator);
		double base_3 = base_2 + noise_3;

		double noise_4 = 2.0 * in_standard_deviation * noise_distribution(generator);
		double base_4 = base_3 + noise_4;

		end_mean = 0.999*end_mean + 0.001*base_4;
		double end_curr_variance = (base_4 - end_mean) * (base_4 - end_mean);
		end_variance = 0.999*end_variance + 0.001*end_curr_variance;

		vector<double> inputs_4{base_4};
		noise_network_4->activate(inputs_4);
		double predicted_noise_4 = noise_network_4->output->acti_vals[0];
		vector<double> errors_4(1);
		errors_4[0] = noise_4 - predicted_noise_4;
		noise_network_4->backprop(errors_4);

		double predicted_base_3 = base_4 - predicted_noise_4;

		{
			vector<double> inputs_3{base_3};
			noise_network_3->activate(inputs_3);
			double predicted_noise_3 = noise_network_3->output->acti_vals[0];
			vector<double> errors_3(1);
			errors_3[0] = noise_3 - predicted_noise_3;
			noise_network_3->backprop(errors_3);
		}

		vector<double> inputs_3{predicted_base_3};
		noise_network_3->activate(inputs_3);
		double predicted_noise_3 = noise_network_3->output->acti_vals[0];

		double predicted_base_2 = predicted_base_3 - predicted_noise_3;

		{
			vector<double> inputs_2{base_2};
			noise_network_2->activate(inputs_2);
			double predicted_noise_2 = noise_network_2->output->acti_vals[0];
			vector<double> errors_2(1);
			errors_2[0] = noise_2 - predicted_noise_2;
			noise_network_2->backprop(errors_2);
		}

		vector<double> inputs_2{predicted_base_2};
		noise_network_2->activate(inputs_2);
		double predicted_noise_2 = noise_network_2->output->acti_vals[0];

		double predicted_base_1 = predicted_base_2 - predicted_noise_2;

		{
			vector<double> inputs_1{base_1};
			noise_network_1->activate(inputs_1);
			double predicted_noise_1 = noise_network_1->output->acti_vals[0];
			vector<double> errors_1(4);
			errors_1[0] = noise_1 - predicted_noise_1;
			noise_network_1->backprop(errors_1);
		}

		vector<double> inputs_1{predicted_base_1};
		noise_network_1->activate(inputs_1);
		double predicted_noise_1 = noise_network_1->output->acti_vals[0];

		double predicted_base = predicted_base_1 - predicted_noise_1;

		if ((iter_index + 1) % 100000 == 0) {
			cout << iter_index << endl;
			cout << "base: " << base << endl;
			cout << "base_4: " << base_4 << endl;
			cout << "predicted_base: " << predicted_base << endl;
			cout << endl;
		}
	}

	cout << "in_mean: " << in_mean << endl;
	cout << "in_variance: " << in_variance << endl;
	cout << "end_mean: " << end_mean << endl;
	cout << "end_variance: " << end_variance << endl;

	double end_standard_deviation = sqrt(end_variance);
	normal_distribution<double> init_end_distribution(end_mean, end_standard_deviation);
	for (int iter_index = 0; iter_index < 40; iter_index++) {
		cout << iter_index << endl;

		double start = init_end_distribution(generator);
		cout << "start: " << start << endl;

		vector<double> inputs_4{start};
		noise_network_4->activate(inputs_4);
		double predicted_noise_4 = noise_network_4->output->acti_vals[0];

		double predicted_base_3 = start - predicted_noise_4;

		vector<double> inputs_3{predicted_base_3};
		noise_network_3->activate(inputs_3);
		double predicted_noise_3 = noise_network_3->output->acti_vals[0];

		double predicted_base_2 = predicted_base_3 - predicted_noise_3;

		vector<double> inputs_2{predicted_base_2};
		noise_network_2->activate(inputs_2);
		double predicted_noise_2 = noise_network_2->output->acti_vals[0];

		double predicted_base_1 = predicted_base_2 - predicted_noise_2;

		vector<double> inputs_1{predicted_base_1};
		noise_network_1->activate(inputs_1);
		double predicted_noise_1 = noise_network_1->output->acti_vals[0];

		double predicted_base = predicted_base_1 - predicted_noise_1;

		cout << "predicted_base: " << predicted_base << endl;

		cout << endl;
	}

	cout << "Done" << endl;
}
