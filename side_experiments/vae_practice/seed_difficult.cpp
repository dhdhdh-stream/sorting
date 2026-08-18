#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "network.h"

using namespace std;

int seed;

default_random_engine generator;

// const int DENOISE_NUM_LAYERS = 5;
// const vector<double> NOISE_MULTIPLIERS{
// 	0.05, 0.25, 0.45, 0.65, 0.85
// };
// const int DENOISE_NUM_LAYERS = 4;
// const vector<double> NOISE_MULTIPLIERS{
// 	0.05, 0.3, 0.55, 0.8
// };
const int DENOISE_NUM_LAYERS = 4;
const vector<double> NOISE_MULTIPLIERS{
	0.05, 0.25, 0.45, 0.65
};

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	/**
	 * - inputs:
	 *   - input
	 *   - obs[0]
	 *   - obs[1]
	 * 
	 * - outputs:
	 *   - positive_predicted_noise[0]
	 *   - negative_predicted_noise[0]
	 *   - noise_direction[0]
	 *   - positive_predicted_noise[1]
	 *   - negative_predicted_noise[1]
	 *   - noise_direction[1]
	 */
	vector<Network*> networks(DENOISE_NUM_LAYERS);
	for (int layer_index = 0; layer_index < DENOISE_NUM_LAYERS; layer_index++) {
		networks[layer_index] = new Network(3, 6);
	}

	double in_mean_0 = 0.0;
	double in_variance_0 = 1.0;
	double in_mean_1 = 0.0;
	double in_variance_1 = 1.0;

	uniform_int_distribution<int> type_distribution(0, 1);
	uniform_int_distribution<int> base_0_distribution(0, 3);
	uniform_int_distribution<int> base_1_distribution(0, 1);
	normal_distribution<double> noise_distribution(0, 1);
	uniform_real_distribution<double> direction_distribution(-1.0, 1.0);
	// for (int iter_index = 0; iter_index < 1000000; iter_index++) {
	for (int iter_index = 0; iter_index < 3000000; iter_index++) {
		int type = type_distribution(generator);

		double base_0;
		double base_1;
		if (type == 0) {
			if (base_0_distribution(generator) == 0) {
				base_0 = 1.0;
				base_1 = -1.0;
			} else {
				base_0 = -1.0;
				base_1 = 1.0;
			}
		} else {
			if (base_1_distribution(generator) == 0) {
				base_0 = 2.0;
				base_1 = 2.0;
			} else {
				base_0 = -0.3;
				base_1 = 0.3;
			}
		}

		in_mean_0 = 0.999*in_mean_0 + 0.001*base_0;
		double in_curr_variance_0 = (base_0 - in_mean_0) * (base_0 - in_mean_0);
		in_variance_0 = 0.999*in_variance_0 + 0.001*in_curr_variance_0;
		in_mean_1 = 0.999*in_mean_1 + 0.001*base_1;
		double in_curr_variance_1 = (base_1 - in_mean_1) * (base_1 - in_mean_1);
		in_variance_1 = 0.999*in_variance_1 + 0.001*in_curr_variance_1;

		double in_standard_deviation_0 = sqrt(in_variance_0);
		double in_standard_deviation_1 = sqrt(in_variance_1);

		for (int layer_index = 0; layer_index < DENOISE_NUM_LAYERS; layer_index++) {
			double noise_0 = NOISE_MULTIPLIERS[layer_index] * in_standard_deviation_0 * noise_distribution(generator);
			double noise_1 = NOISE_MULTIPLIERS[layer_index] * in_standard_deviation_1 * noise_distribution(generator);

			vector<double> inputs{
				(double)type,
				base_0 + noise_0,
				base_1 + noise_1,
				(double)layer_index
			};
			networks[layer_index]->activate(inputs);
			double positive_noise_0 = networks[layer_index]->output->acti_vals[0];
			double negative_noise_0 = networks[layer_index]->output->acti_vals[1];
			double noise_direction_0 = networks[layer_index]->output->acti_vals[2];
			double positive_noise_1 = networks[layer_index]->output->acti_vals[3];
			double negative_noise_1 = networks[layer_index]->output->acti_vals[4];
			double noise_direction_1 = networks[layer_index]->output->acti_vals[5];
			vector<double> errors(6);
			if (noise_0 > 0.0) {
				errors[0] = noise_0 - positive_noise_0;
				errors[1] = 0.0;
				if (noise_direction_0 >= 1.0) {
					errors[2] = 0.0;
				} else {
					errors[2] = 1.0 - noise_direction_0;
				}
			} else {
				errors[0] = 0.0;
				errors[1] = noise_0 - negative_noise_0;
				if (noise_direction_0 <= -1.0) {
					errors[2] = 0.0;
				} else {
					errors[2] = -1.0 - noise_direction_0;
				}
			}
			if (noise_1 > 0.0) {
				errors[3] = noise_1 - positive_noise_1;
				errors[4] = 0.0;
				if (noise_direction_1 >= 1.0) {
					errors[5] = 0.0;
				} else {
					errors[5] = 1.0 - noise_direction_1;
				}
			} else {
				errors[3] = 0.0;
				errors[4] = noise_1 - negative_noise_1;
				if (noise_direction_1 <= -1.0) {
					errors[5] = 0.0;
				} else {
					errors[5] = -1.0 - noise_direction_1;
				}
			}
			networks[layer_index]->backprop(errors);
		}

		if ((iter_index + 1) % 100000 == 0) {
			cout << iter_index << endl;
			cout << endl;
		}
	}

	vector<vector<double>> seeds;
	for (int s_index = 0; s_index < 1000; s_index++) {
		int type = type_distribution(generator);

		double base_0;
		double base_1;
		if (type == 0) {
			if (base_0_distribution(generator) == 0) {
				base_0 = 1.0;
				base_1 = -1.0;
			} else {
				base_0 = -1.0;
				base_1 = 1.0;
			}
		} else {
			if (base_1_distribution(generator) == 0) {
				base_0 = 2.0;
				base_1 = 2.0;
			} else {
				base_0 = -0.3;
				base_1 = 0.3;
			}
		}
		seeds.push_back({base_0, base_1});
	}

	uniform_int_distribution<int> seed_distribution(0, seeds.size()-1);
	for (int iter_index = 0; iter_index < 40; iter_index++) {
		cout << iter_index << endl;

		int type = type_distribution(generator);
		cout << "type: " << type << endl;

		int seed_index = seed_distribution(generator);
		double curr_0 = seeds[seed_index][0];
		cout << "curr_0: " << curr_0 << endl;
		double curr_1 = seeds[seed_index][1];
		cout << "curr_1: " << curr_1 << endl;

		for (int layer_index = DENOISE_NUM_LAYERS-1; layer_index >= 0; layer_index--) {
			vector<double> inputs{(double)type, curr_0, curr_1, (double)layer_index};
			networks[layer_index]->activate(inputs);
			double positive_noise_0 = networks[layer_index]->output->acti_vals[0];
			cout << "positive_noise_0: " << positive_noise_0 << endl;
			double negative_noise_0 = networks[layer_index]->output->acti_vals[1];
			cout << "negative_noise_0: " << negative_noise_0 << endl;
			double noise_direction_0 = networks[layer_index]->output->acti_vals[2];
			cout << "noise_direction_0: " << noise_direction_0 << endl;
			double positive_noise_1 = networks[layer_index]->output->acti_vals[3];
			cout << "positive_noise_1: " << positive_noise_1 << endl;
			double negative_noise_1 = networks[layer_index]->output->acti_vals[4];
			cout << "negative_noise_1: " << negative_noise_1 << endl;
			double noise_direction_1 = networks[layer_index]->output->acti_vals[5];
			cout << "noise_direction_1: " << noise_direction_1 << endl;

			if (noise_direction_0 >= 1.0) {
				curr_0 = curr_0 - positive_noise_0;
			} else if (noise_direction_0 <= -1.0) {
				curr_0 = curr_0 - negative_noise_0;
			} else {
				double rand_val = direction_distribution(generator);
				if (rand_val > noise_direction_0) {
					curr_0 = curr_0 - negative_noise_0;
				} else {
					curr_0 = curr_0 - positive_noise_0;
				}
			}
			cout << "curr_0: " << curr_0 << endl;
			if (noise_direction_1 >= 1.0) {
				curr_1 = curr_1 - positive_noise_1;
			} else if (noise_direction_1 <= -1.0) {
				curr_1 = curr_1 - negative_noise_1;
			} else {
				double rand_val = direction_distribution(generator);
				if (rand_val > noise_direction_1) {
					curr_1 = curr_1 - negative_noise_1;
				} else {
					curr_1 = curr_1 - positive_noise_1;
				}
			}
			cout << "curr_1: " << curr_1 << endl;
		}

		cout << endl;
	}

	cout << "Done" << endl;
}
