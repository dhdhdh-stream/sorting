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

	/**
	 * - predicted_noise
	 * - positive_predicted_error
	 * - negative_predicted_error
	 * - error_direction
	 */
	Network* noise_network_1 = new Network(1, 4);
	Network* noise_network_2 = new Network(1, 4);
	Network* noise_network_3 = new Network(1, 4);
	Network* noise_network_4 = new Network(1, 4);

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
		double positive_error_4 = noise_network_4->output->acti_vals[1];
		double negative_error_4 = noise_network_4->output->acti_vals[2];
		double error_direction_4 = noise_network_4->output->acti_vals[3];
		vector<double> errors_4(4);
		errors_4[0] = noise_4 - predicted_noise_4;
		if (noise_4 > predicted_noise_4) {
			errors_4[1] = (noise_4 - predicted_noise_4) - positive_error_4;
			errors_4[2] = 0.0;
			if (error_direction_4 >= 1.0) {
				errors_4[3] = 0.0;
			} else {
				errors_4[3] = 1.0 - error_direction_4;
			}
		} else {
			errors_4[1] = 0.0;
			errors_4[2] = (noise_4 - predicted_noise_4) - negative_error_4;
			if (error_direction_4 <= -1.0) {
				errors_4[3] = 0.0;
			} else {
				errors_4[3] = -1.0 - error_direction_4;
			}
		}
		noise_network_4->backprop(errors_4);

		double predicted_base_3 = base_4 - predicted_noise_4;
		if (error_direction_4 >= 1.0) {
			predicted_base_3 += positive_error_4;
		} else if (error_direction_4 <= -1.0) {
			predicted_base_3 += negative_error_4;
		} else {
			double rand_val = direction_distribution(generator);
			if (rand_val > error_direction_4) {
				predicted_base_3 += negative_error_4;
			} else {
				predicted_base_3 += positive_error_4;
			}
		}

		{
			vector<double> inputs_3{base_3};
			noise_network_3->activate(inputs_3);
			double predicted_noise_3 = noise_network_3->output->acti_vals[0];
			double positive_error_3 = noise_network_3->output->acti_vals[1];
			double negative_error_3 = noise_network_3->output->acti_vals[2];
			double error_direction_3 = noise_network_3->output->acti_vals[3];
			vector<double> errors_3(4);
			errors_3[0] = noise_3 - predicted_noise_3;
			if (noise_3 > predicted_noise_3) {
				errors_3[1] = (noise_3 - predicted_noise_3) - positive_error_3;
				errors_3[2] = 0.0;
				if (error_direction_3 >= 1.0) {
					errors_3[3] = 0.0;
				} else {
					errors_3[3] = 1.0 - error_direction_3;
				}
			} else {
				errors_3[1] = 0.0;
				errors_3[2] = (noise_3 - predicted_noise_3) - negative_error_3;
				if (error_direction_3 <= -1.0) {
					errors_3[3] = 0.0;
				} else {
					errors_3[3] = -1.0 - error_direction_3;
				}
			}
			noise_network_3->backprop(errors_3);
		}

		vector<double> inputs_3{predicted_base_3};
		noise_network_3->activate(inputs_3);
		double predicted_noise_3 = noise_network_3->output->acti_vals[0];
		double positive_error_3 = noise_network_3->output->acti_vals[1];
		double negative_error_3 = noise_network_3->output->acti_vals[2];
		double error_direction_3 = noise_network_3->output->acti_vals[3];

		double predicted_base_2 = predicted_base_3 - predicted_noise_3;
		if (error_direction_3 >= 1.0) {
			predicted_base_2 += positive_error_3;
		} else if (error_direction_3 <= -1.0) {
			predicted_base_2 += negative_error_3;
		} else {
			double rand_val = direction_distribution(generator);
			if (rand_val > error_direction_3) {
				predicted_base_2 += negative_error_3;
			} else {
				predicted_base_2 += positive_error_3;
			}
		}

		{
			vector<double> inputs_2{base_2};
			noise_network_2->activate(inputs_2);
			double predicted_noise_2 = noise_network_2->output->acti_vals[0];
			double positive_error_2 = noise_network_2->output->acti_vals[1];
			double negative_error_2 = noise_network_2->output->acti_vals[2];
			double error_direction_2 = noise_network_2->output->acti_vals[3];
			vector<double> errors_2(4);
			errors_2[0] = noise_2 - predicted_noise_2;
			if (noise_2 > predicted_noise_2) {
				errors_2[1] = (noise_2 - predicted_noise_2) - positive_error_2;
				errors_2[2] = 0.0;
				if (error_direction_2 >= 1.0) {
					errors_2[3] = 0.0;
				} else {
					errors_2[3] = 1.0 - error_direction_2;
				}
			} else {
				errors_2[1] = 0.0;
				errors_2[2] = (noise_2 - predicted_noise_2) - negative_error_2;
				if (error_direction_2 <= -1.0) {
					errors_2[3] = 0.0;
				} else {
					errors_2[3] = -1.0 - error_direction_2;
				}
			}
			noise_network_2->backprop(errors_2);
		}

		vector<double> inputs_2{predicted_base_2};
		noise_network_2->activate(inputs_2);
		double predicted_noise_2 = noise_network_2->output->acti_vals[0];
		double positive_error_2 = noise_network_2->output->acti_vals[1];
		double negative_error_2 = noise_network_2->output->acti_vals[2];
		double error_direction_2 = noise_network_2->output->acti_vals[3];

		double predicted_base_1 = predicted_base_2 - predicted_noise_2;
		if (error_direction_2 >= 1.0) {
			predicted_base_1 += positive_error_2;
		} else if (error_direction_2 <= -1.0) {
			predicted_base_1 += negative_error_2;
		} else {
			double rand_val = direction_distribution(generator);
			if (rand_val > error_direction_2) {
				predicted_base_1 += negative_error_2;
			} else {
				predicted_base_1 += positive_error_2;
			}
		}

		{
			vector<double> inputs_1{base_1};
			noise_network_1->activate(inputs_1);
			double predicted_noise_1 = noise_network_1->output->acti_vals[0];
			double positive_error_1 = noise_network_1->output->acti_vals[1];
			double negative_error_1 = noise_network_1->output->acti_vals[2];
			double error_direction_1 = noise_network_1->output->acti_vals[3];
			vector<double> errors_1(4);
			errors_1[0] = noise_1 - predicted_noise_1;
			if (noise_1 > predicted_noise_1) {
				errors_1[1] = (noise_1 - predicted_noise_1) - positive_error_1;
				errors_1[2] = 0.0;
				if (error_direction_1 >= 1.0) {
					errors_1[3] = 0.0;
				} else {
					errors_1[3] = 1.0 - error_direction_1;
				}
			} else {
				errors_1[1] = 0.0;
				errors_1[2] = (noise_1 - predicted_noise_1) - negative_error_1;
				if (error_direction_1 <= -1.0) {
					errors_1[3] = 0.0;
				} else {
					errors_1[3] = -1.0 - error_direction_1;
				}
			}
			noise_network_1->backprop(errors_1);
		}

		vector<double> inputs_1{predicted_base_1};
		noise_network_1->activate(inputs_1);
		double predicted_noise_1 = noise_network_1->output->acti_vals[0];
		double positive_error_1 = noise_network_1->output->acti_vals[1];
		double negative_error_1 = noise_network_1->output->acti_vals[2];
		double error_direction_1 = noise_network_1->output->acti_vals[3];

		double predicted_base = predicted_base_1 - predicted_noise_1;
		if (error_direction_1 >= 1.0) {
			predicted_base += positive_error_1;
		} else if (error_direction_1 <= -1.0) {
			predicted_base += negative_error_1;
		} else {
			double rand_val = direction_distribution(generator);
			if (rand_val > error_direction_1) {
				predicted_base += negative_error_1;
			} else {
				predicted_base += positive_error_1;
			}
		}

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
		double positive_error_4 = noise_network_4->output->acti_vals[1];
		double negative_error_4 = noise_network_4->output->acti_vals[2];
		double error_direction_4 = noise_network_4->output->acti_vals[3];

		double predicted_base_3 = start - predicted_noise_4;
		if (error_direction_4 >= 1.0) {
			predicted_base_3 += positive_error_4;
		} else if (error_direction_4 <= -1.0) {
			predicted_base_3 += negative_error_4;
		} else {
			double rand_val = direction_distribution(generator);
			if (rand_val > error_direction_4) {
				predicted_base_3 += negative_error_4;
			} else {
				predicted_base_3 += positive_error_4;
			}
		}

		vector<double> inputs_3{predicted_base_3};
		noise_network_3->activate(inputs_3);
		double predicted_noise_3 = noise_network_3->output->acti_vals[0];
		double positive_error_3 = noise_network_3->output->acti_vals[1];
		double negative_error_3 = noise_network_3->output->acti_vals[2];
		double error_direction_3 = noise_network_3->output->acti_vals[3];

		double predicted_base_2 = predicted_base_3 - predicted_noise_3;
		if (error_direction_3 >= 1.0) {
			predicted_base_2 += positive_error_3;
		} else if (error_direction_3 <= -1.0) {
			predicted_base_2 += negative_error_3;
		} else {
			double rand_val = direction_distribution(generator);
			if (rand_val > error_direction_3) {
				predicted_base_2 += negative_error_3;
			} else {
				predicted_base_2 += positive_error_3;
			}
		}

		vector<double> inputs_2{predicted_base_2};
		noise_network_2->activate(inputs_2);
		double predicted_noise_2 = noise_network_2->output->acti_vals[0];
		double positive_error_2 = noise_network_2->output->acti_vals[1];
		double negative_error_2 = noise_network_2->output->acti_vals[2];
		double error_direction_2 = noise_network_2->output->acti_vals[3];

		double predicted_base_1 = predicted_base_2 - predicted_noise_2;
		if (error_direction_2 >= 1.0) {
			predicted_base_1 += positive_error_2;
		} else if (error_direction_2 <= -1.0) {
			predicted_base_1 += negative_error_2;
		} else {
			double rand_val = direction_distribution(generator);
			if (rand_val > error_direction_2) {
				predicted_base_1 += negative_error_2;
			} else {
				predicted_base_1 += positive_error_2;
			}
		}

		vector<double> inputs_1{predicted_base_1};
		noise_network_1->activate(inputs_1);
		double predicted_noise_1 = noise_network_1->output->acti_vals[0];
		double positive_error_1 = noise_network_1->output->acti_vals[1];
		double negative_error_1 = noise_network_1->output->acti_vals[2];
		double error_direction_1 = noise_network_1->output->acti_vals[3];

		double predicted_base = predicted_base_1 - predicted_noise_1;
		if (error_direction_1 >= 1.0) {
			predicted_base += positive_error_1;
		} else if (error_direction_1 <= -1.0) {
			predicted_base += negative_error_1;
		} else {
			double rand_val = direction_distribution(generator);
			if (rand_val > error_direction_1) {
				predicted_base += negative_error_1;
			} else {
				predicted_base += positive_error_1;
			}
		}
		cout << "predicted_base: " << predicted_base << endl;

		cout << endl;
	}

	cout << "Done" << endl;
}
