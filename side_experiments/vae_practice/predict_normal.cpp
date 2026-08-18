#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "network.h"

using namespace std;

int seed;

default_random_engine generator;

// const int DENOISE_NUM_LAYERS = 4;
// const vector<double> NOISE_MULTIPLIERS{
// 	0.05, 0.1, 0.15, 0.2
// };
// const int DENOISE_NUM_LAYERS = 4;
// const vector<double> NOISE_MULTIPLIERS{
// 	0.05, 0.25, 0.45, 0.65
// };
// const int DENOISE_NUM_LAYERS = 4;
// const vector<double> NOISE_MULTIPLIERS{
// 	0.05, 0.1, 0.2, 0.4
// };
const int DENOISE_NUM_LAYERS = 4;
const vector<double> NOISE_MULTIPLIERS{
	0.05, 0.2, 0.35, 0.5
};
// - seems good

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	Network* normal_network = new Network(1, 4);

	/**
	 * - inputs:
	 *   - input
	 *   - obs[0]
	 *   - obs[1]
	 * 
	 * - outputs:
	 *   - predicted_noise[0]
	 *   - predicted_noise[1]
	 */
	vector<Network*> networks(DENOISE_NUM_LAYERS);
	for (int layer_index = 0; layer_index < DENOISE_NUM_LAYERS; layer_index++) {
		networks[layer_index] = new Network(3, 2);
	}

	uniform_int_distribution<int> type_distribution(0, 1);
	uniform_int_distribution<int> base_0_distribution(0, 3);
	uniform_int_distribution<int> base_1_distribution(0, 1);
	normal_distribution<double> noise_distribution(0, 1);
	for (int iter_index = 0; iter_index < 5000000; iter_index++) {
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

		vector<double> normal_inputs{
			(double)type,
		};
		normal_network->activate(normal_inputs);

		double predicted_mean_0 = normal_network->output->acti_vals[0];
		double predicted_deviation_0 = normal_network->output->acti_vals[1];
		double predicted_mean_1 = normal_network->output->acti_vals[2];
		double predicted_deviation_1 = normal_network->output->acti_vals[3];

		vector<double> normal_errors(4);
		normal_errors[0] = base_0 - predicted_mean_0;
		normal_errors[1] = abs(base_0 - predicted_mean_0) - predicted_deviation_0;
		normal_errors[2] = base_1 - predicted_mean_1;
		normal_errors[3] = abs(base_1 - predicted_mean_1) - predicted_deviation_1;
		normal_network->backprop(normal_errors);

		for (int layer_index = 0; layer_index < DENOISE_NUM_LAYERS; layer_index++) {
			double noise_0 = NOISE_MULTIPLIERS[layer_index] * predicted_deviation_0 * noise_distribution(generator);
			double noise_1 = NOISE_MULTIPLIERS[layer_index] * predicted_deviation_1 * noise_distribution(generator);

			vector<double> inputs{
				(double)type,
				base_0 + noise_0,
				base_1 + noise_1,
			};
			networks[layer_index]->activate(inputs);
			double predicted_noise_0 = networks[layer_index]->output->acti_vals[0];
			double predicted_noise_1 = networks[layer_index]->output->acti_vals[1];
			vector<double> errors(2);
			errors[0] = noise_0 - predicted_noise_0;
			errors[1] = noise_1 - predicted_noise_1;
			networks[layer_index]->backprop(errors);
		}

		if ((iter_index + 1) % 100000 == 0) {
			cout << iter_index << endl;

			// cout << "type: " << type << endl;
			// cout << "predicted_mean_0: " << predicted_mean_0 << endl;
			// cout << "predicted_deviation_0: " << predicted_deviation_0 << endl;
			// cout << "predicted_mean_1: " << predicted_mean_1 << endl;
			// cout << "predicted_deviation_1: " << predicted_deviation_1 << endl;

			cout << endl;
		}
	}

	for (int iter_index = 0; iter_index < 40; iter_index++) {
		cout << iter_index << endl;

		int type = type_distribution(generator);
		cout << "type: " << type << endl;

		vector<double> normal_inputs{
			(double)type,
		};
		normal_network->activate(normal_inputs);

		double predicted_mean_0 = normal_network->output->acti_vals[0];
		double predicted_deviation_0 = normal_network->output->acti_vals[1];
		double predicted_mean_1 = normal_network->output->acti_vals[2];
		double predicted_deviation_1 = normal_network->output->acti_vals[3];

		double curr_0 = predicted_mean_0 + noise_distribution(generator) * predicted_deviation_0;
		cout << "curr_0: " << curr_0 << endl;
		double curr_1 = predicted_mean_1 + noise_distribution(generator) * predicted_deviation_1;
		cout << "curr_1: " << curr_1 << endl;

		for (int layer_index = DENOISE_NUM_LAYERS-1; layer_index >= 0; layer_index--) {
			vector<double> inputs{(double)type, curr_0, curr_1};
			networks[layer_index]->activate(inputs);
			double predicted_noise_0 = networks[layer_index]->output->acti_vals[0];
			cout << "predicted_noise_0: " << predicted_noise_0 << endl;
			double predicted_noise_1 = networks[layer_index]->output->acti_vals[1];
			cout << "predicted_noise_1: " << predicted_noise_1 << endl;

			curr_0 = curr_0 - predicted_noise_0;
			cout << "curr_0: " << curr_0 << endl;
			curr_1 = curr_1 - predicted_noise_1;
			cout << "curr_1: " << curr_1 << endl;
		}

		cout << endl;
	}

	cout << "Done" << endl;
}
