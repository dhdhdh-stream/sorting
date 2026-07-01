// TODO: try more targets than 2
// - but probably don't need more than 4?
//   - beyond 4, probably essentially continuous anyways
//     - at least as seen by networks using as input

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
	 * - inputs:
	 *   - obs
	 *   - layer
	 * 
	 * - outputs:
	 *   - positive_predicted_noise
	 *   - negative_predicted_noise
	 *   - noise_direction
	 */
	Network* noise_network = new Network(2, 3);

	double in_mean = 0.0;
	double in_variance = 1.0;
	double end_mean = 0.0;
	double end_variance = 1.0;

	uniform_int_distribution<int> base_distribution(0, 1);
	normal_distribution<double> noise_distribution(0, 1);
	uniform_real_distribution<double> direction_distribution(-1.0, 1.0);
	// for (int iter_index = 0; iter_index < 1000000; iter_index++) {
	for (int iter_index = 0; iter_index < 3000000; iter_index++) {
		double base = -0.5 + base_distribution(generator);

		in_mean = 0.999*in_mean + 0.001*base;
		double in_curr_variance = (base - in_mean) * (base - in_mean);
		in_variance = 0.999*in_variance + 0.001*in_curr_variance;

		double in_standard_deviation = sqrt(in_variance);

		// double noise_1 = 0.1 * in_standard_deviation * noise_distribution(generator);
		// double noise_1 = 0.2 * in_standard_deviation * noise_distribution(generator);
		double noise_1 = 0.1 * in_standard_deviation * noise_distribution(generator);
		double base_1 = base + noise_1;

		// double noise_2 = 0.3 * in_standard_deviation * noise_distribution(generator);
		// double noise_2 = 0.5 * in_standard_deviation * noise_distribution(generator);
		// double noise_2 = 0.4 * in_standard_deviation * noise_distribution(generator);
		double noise_2 = 0.2 * in_standard_deviation * noise_distribution(generator);
		double base_2 = base_1 + noise_2;

		// double noise_3 = 0.8 * in_standard_deviation * noise_distribution(generator);
		// double noise_3 = in_standard_deviation * noise_distribution(generator);
		// double noise_3 = 0.8 * in_standard_deviation * noise_distribution(generator);
		double noise_3 = 0.4 * in_standard_deviation * noise_distribution(generator);
		double base_3 = base_2 + noise_3;

		// double noise_4 = 2.0 * in_standard_deviation * noise_distribution(generator);
		// double noise_4 = 1.6 * in_standard_deviation * noise_distribution(generator);
		double noise_4 = 0.8 * in_standard_deviation * noise_distribution(generator);
		double base_4 = base_3 + noise_4;

		end_mean = 0.999*end_mean + 0.001*base_4;
		double end_curr_variance = (base_4 - end_mean) * (base_4 - end_mean);
		end_variance = 0.999*end_variance + 0.001*end_curr_variance;

		// vector<double> inputs_4{base_4, 4.0};
		vector<double> inputs_4{base_4, 1.5};
		noise_network->activate(inputs_4);
		double positive_noise_4 = noise_network->output->acti_vals[0];
		double negative_noise_4 = noise_network->output->acti_vals[1];
		double noise_direction_4 = noise_network->output->acti_vals[2];
		vector<double> errors_4(3);
		if (noise_4 > 0.0) {
			errors_4[0] = noise_4 - positive_noise_4;
			errors_4[1] = 0.0;
			if (noise_direction_4 >= 1.0) {
				errors_4[2] = 0.0;
			} else {
				errors_4[2] = 1.0 - noise_direction_4;
			}
		} else {
			errors_4[0] = 0.0;
			errors_4[1] = noise_4 - negative_noise_4;
			if (noise_direction_4 <= -1.0) {
				errors_4[2] = 0.0;
			} else {
				errors_4[2] = -1.0 - noise_direction_4;
			}
		}
		noise_network->backprop(errors_4);

		double predicted_base_3;
		if (noise_direction_4 >= 1.0) {
			predicted_base_3 = base_4 - positive_noise_4;
		} else if (noise_direction_4 <= -1.0) {
			predicted_base_3 = base_4 - negative_noise_4;
		} else {
			double rand_val = direction_distribution(generator);
			if (rand_val > noise_direction_4) {
				predicted_base_3 = base_4 - negative_noise_4;
			} else {
				predicted_base_3 = base_4 - positive_noise_4;
			}
		}

		{
			// vector<double> inputs_3{base_3, 3.0};
			vector<double> inputs_3{base_3, 0.5};
			noise_network->activate(inputs_3);
			double positive_noise_3 = noise_network->output->acti_vals[0];
			double negative_noise_3 = noise_network->output->acti_vals[1];
			double noise_direction_3 = noise_network->output->acti_vals[2];
			vector<double> errors_3(3);
			if (noise_3 > 0.0) {
				errors_3[0] = noise_3 - positive_noise_3;
				errors_3[1] = 0.0;
				if (noise_direction_3 >= 1.0) {
					errors_3[2] = 0.0;
				} else {
					errors_3[2] = 1.0 - noise_direction_3;
				}
			} else {
				errors_3[0] = 0.0;
				errors_3[1] = noise_3 - negative_noise_3;
				if (noise_direction_3 <= -1.0) {
					errors_3[2] = 0.0;
				} else {
					errors_3[2] = -1.0 - noise_direction_3;
				}
			}
			noise_network->backprop(errors_3);
		}

		// vector<double> inputs_3{predicted_base_3, 3.0};
		vector<double> inputs_3{predicted_base_3, 0.5};
		noise_network->activate(inputs_3);
		double positive_noise_3 = noise_network->output->acti_vals[0];
		double negative_noise_3 = noise_network->output->acti_vals[1];
		double noise_direction_3 = noise_network->output->acti_vals[2];

		double predicted_base_2;
		if (noise_direction_3 >= 1.0) {
			predicted_base_2 = predicted_base_3 - positive_noise_3;
		} else if (noise_direction_3 <= -1.0) {
			predicted_base_2 = predicted_base_3 - negative_noise_3;
		} else {
			double rand_val = direction_distribution(generator);
			if (rand_val > noise_direction_3) {
				predicted_base_2 = predicted_base_3 - negative_noise_3;
			} else {
				predicted_base_2 = predicted_base_3 - positive_noise_3;
			}
		}

		{
			// vector<double> inputs_2{base_2, 2.0};
			vector<double> inputs_2{base_2, -0.5};
			noise_network->activate(inputs_2);
			double positive_noise_2 = noise_network->output->acti_vals[0];
			double negative_noise_2 = noise_network->output->acti_vals[1];
			double noise_direction_2 = noise_network->output->acti_vals[2];
			vector<double> errors_2(3);
			if (noise_2 > 0.0) {
				errors_2[0] = noise_2 - positive_noise_2;
				errors_2[1] = 0.0;
				if (noise_direction_2 >= 1.0) {
					errors_2[2] = 0.0;
				} else {
					errors_2[2] = 1.0 - noise_direction_2;
				}
			} else {
				errors_2[0] = 0.0;
				errors_2[1] = noise_2 - negative_noise_2;
				if (noise_direction_2 <= -1.0) {
					errors_2[2] = 0.0;
				} else {
					errors_2[2] = -1.0 - noise_direction_2;
				}
			}
			noise_network->backprop(errors_2);
		}

		// vector<double> inputs_2{predicted_base_2, 2.0};
		vector<double> inputs_2{predicted_base_2, -0.5};
		noise_network->activate(inputs_2);
		double positive_noise_2 = noise_network->output->acti_vals[0];
		double negative_noise_2 = noise_network->output->acti_vals[1];
		double noise_direction_2 = noise_network->output->acti_vals[2];

		double predicted_base_1;
		if (noise_direction_2 >= 1.0) {
			predicted_base_1 = predicted_base_2 - positive_noise_2;
		} else if (noise_direction_2 <= -1.0) {
			predicted_base_1 = predicted_base_2 - negative_noise_2;
		} else {
			double rand_val = direction_distribution(generator);
			if (rand_val > noise_direction_2) {
				predicted_base_1 = predicted_base_2 - negative_noise_2;
			} else {
				predicted_base_1 = predicted_base_2 - positive_noise_2;
			}
		}

		{
			// vector<double> inputs_1{base_1, 1.0};
			vector<double> inputs_1{base_1, -1.5};
			noise_network->activate(inputs_1);
			double positive_noise_1 = noise_network->output->acti_vals[0];
			double negative_noise_1 = noise_network->output->acti_vals[1];
			double noise_direction_1 = noise_network->output->acti_vals[2];
			vector<double> errors_1(3);
			if (noise_1 > 0.0) {
				errors_1[0] = noise_1 - positive_noise_1;
				errors_1[1] = 0.0;
				if (noise_direction_1 >= 1.0) {
					errors_1[2] = 0.0;
				} else {
					errors_1[2] = 1.0 - noise_direction_1;
				}
			} else {
				errors_1[0] = 0.0;
				errors_1[1] = noise_1 - negative_noise_1;
				if (noise_direction_1 <= -1.0) {
					errors_1[2] = 0.0;
				} else {
					errors_1[2] = -1.0 - noise_direction_1;
				}
			}
			noise_network->backprop(errors_1);
		}

		// vector<double> inputs_1{predicted_base_1, 1.0};
		vector<double> inputs_1{predicted_base_1, -1.5};
		noise_network->activate(inputs_1);
		double positive_noise_1 = noise_network->output->acti_vals[0];
		double negative_noise_1 = noise_network->output->acti_vals[1];
		double noise_direction_1 = noise_network->output->acti_vals[2];

		double predicted_base;
		if (noise_direction_1 >= 1.0) {
			predicted_base = predicted_base_1 - positive_noise_1;
		} else if (noise_direction_1 <= -1.0) {
			predicted_base = predicted_base_1 - negative_noise_1;
		} else {
			double rand_val = direction_distribution(generator);
			if (rand_val > noise_direction_1) {
				predicted_base = predicted_base_1 - negative_noise_1;
			} else {
				predicted_base = predicted_base_1 - positive_noise_1;
			}
		}

		if ((iter_index + 1) % 100000 == 0) {
			cout << iter_index << endl;
			cout << "base: " << base << endl;
			cout << "base_4: " << base_4 << endl;
			cout << "noise_4: " << noise_4 << endl;
			cout << "positive_noise_4: " << positive_noise_4 << endl;
			cout << "negative_noise_4: " << negative_noise_4 << endl;
			cout << "noise_direction_4: " << noise_direction_4 << endl;
			cout << "predicted_base_3: " << predicted_base_3 << endl;
			cout << "predicted_base: " << predicted_base << endl;
			cout << endl;
		}
	}

	double in_standard_deviation = sqrt(in_variance);
	double end_standard_deviation = sqrt(end_variance);

	cout << "in_mean: " << in_mean << endl;
	cout << "in_variance: " << in_variance << endl;
	cout << "in_standard_deviation: " << in_standard_deviation << endl;
	cout << "end_mean: " << end_mean << endl;
	cout << "end_variance: " << end_variance << endl;
	cout << "end_standard_deviation: " << end_standard_deviation << endl;

	normal_distribution<double> init_end_distribution(end_mean, end_standard_deviation);
	for (int iter_index = 0; iter_index < 40; iter_index++) {
		cout << iter_index << endl;

		double start = init_end_distribution(generator);
		cout << "start: " << start << endl;

		// vector<double> inputs_4{start, 4.0};
		vector<double> inputs_4{start, 1.5};
		noise_network->activate(inputs_4);
		double positive_noise_4 = noise_network->output->acti_vals[0];
		double negative_noise_4 = noise_network->output->acti_vals[1];
		double noise_direction_4 = noise_network->output->acti_vals[2];

		double predicted_base_3;
		if (noise_direction_4 >= 1.0) {
			predicted_base_3 = start - positive_noise_4;
		} else if (noise_direction_4 <= -1.0) {
			predicted_base_3 = start - negative_noise_4;
		} else {
			double rand_val = direction_distribution(generator);
			if (rand_val > noise_direction_4) {
				predicted_base_3 = start - negative_noise_4;
			} else {
				predicted_base_3 = start - positive_noise_4;
			}
		}

		// vector<double> inputs_3{predicted_base_3, 3.0};
		vector<double> inputs_3{predicted_base_3, 0.5};
		noise_network->activate(inputs_3);
		double positive_noise_3 = noise_network->output->acti_vals[0];
		double negative_noise_3 = noise_network->output->acti_vals[1];
		double noise_direction_3 = noise_network->output->acti_vals[2];

		double predicted_base_2;
		if (noise_direction_3 >= 1.0) {
			predicted_base_2 = predicted_base_3 - positive_noise_3;
		} else if (noise_direction_3 <= -1.0) {
			predicted_base_2 = predicted_base_3 - negative_noise_3;
		} else {
			double rand_val = direction_distribution(generator);
			if (rand_val > noise_direction_3) {
				predicted_base_2 = predicted_base_3 - negative_noise_3;
			} else {
				predicted_base_2 = predicted_base_3 - positive_noise_3;
			}
		}

		// vector<double> inputs_2{predicted_base_2, 2.0};
		vector<double> inputs_2{predicted_base_2, -0.5};
		noise_network->activate(inputs_2);
		double positive_noise_2 = noise_network->output->acti_vals[0];
		double negative_noise_2 = noise_network->output->acti_vals[1];
		double noise_direction_2 = noise_network->output->acti_vals[2];

		double predicted_base_1;
		if (noise_direction_2 >= 1.0) {
			predicted_base_1 = predicted_base_2 - positive_noise_2;
		} else if (noise_direction_2 <= -1.0) {
			predicted_base_1 = predicted_base_2 - negative_noise_2;
		} else {
			double rand_val = direction_distribution(generator);
			if (rand_val > noise_direction_2) {
				predicted_base_1 = predicted_base_2 - negative_noise_2;
			} else {
				predicted_base_1 = predicted_base_2 - positive_noise_2;
			}
		}

		// vector<double> inputs_1{predicted_base_1, 1.0};
		vector<double> inputs_1{predicted_base_1, -1.5};
		noise_network->activate(inputs_1);
		double positive_noise_1 = noise_network->output->acti_vals[0];
		double negative_noise_1 = noise_network->output->acti_vals[1];
		double noise_direction_1 = noise_network->output->acti_vals[2];

		double predicted_base;
		if (noise_direction_1 >= 1.0) {
			predicted_base = predicted_base_1 - positive_noise_1;
		} else if (noise_direction_1 <= -1.0) {
			predicted_base = predicted_base_1 - negative_noise_1;
		} else {
			double rand_val = direction_distribution(generator);
			if (rand_val > noise_direction_1) {
				predicted_base = predicted_base_1 - negative_noise_1;
			} else {
				predicted_base = predicted_base_1 - positive_noise_1;
			}
		}
		cout << "predicted_base: " << predicted_base << endl;

		cout << endl;
	}

	cout << "Done" << endl;
}
