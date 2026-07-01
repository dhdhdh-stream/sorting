// - probably best

// - want as many layers as possible
//   - want min at 0.05, max at 1.6
//     - 0.05, 0.1, 0.2, 0.4, 0.8, 1.6

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
	 *   - input
	 *   - obs[0]
	 *   - obs[1]
	 *   - layer
	 * 
	 * - outputs:
	 *   - positive_predicted_noise[0]
	 *   - negative_predicted_noise[0]
	 *   - noise_direction[0]
	 *   - positive_predicted_noise[1]
	 *   - negative_predicted_noise[1]
	 *   - noise_direction[1]
	 */
	Network* noise_network = new Network(4, 6);

	double in_mean_0 = 0.0;
	double in_variance_0 = 1.0;
	double in_mean_1 = 0.0;
	double in_variance_1 = 1.0;

	uniform_int_distribution<int> type_distribution(0, 1);
	uniform_int_distribution<int> base_distribution(0, 1);
	normal_distribution<double> noise_distribution(0, 1);
	uniform_real_distribution<double> direction_distribution(-1.0, 1.0);
	// for (int iter_index = 0; iter_index < 1000000; iter_index++) {
	for (int iter_index = 0; iter_index < 3000000; iter_index++) {
		int type = type_distribution(generator);

		double base_0;
		double base_1;
		if (type == 0) {
			if (base_distribution(generator) == 0) {
				base_0 = 1.0;
				base_1 = -1.0;
			} else {
				base_0 = -1.0;
				base_1 = 1.0;
			}
		} else {
			if (base_distribution(generator) == 0) {
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

		// double noise_1_0 = 0.2 * in_standard_deviation_0 * noise_distribution(generator);
		double noise_1_0 = 0.1 * in_standard_deviation_0 * noise_distribution(generator);
		double base_1_0 = base_0 + noise_1_0;
		// double noise_1_1 = 0.2 * in_standard_deviation_1 * noise_distribution(generator);
		double noise_1_1 = 0.1 * in_standard_deviation_1 * noise_distribution(generator);
		double base_1_1 = base_1 + noise_1_1;

		// double noise_2_0 = 0.4 * in_standard_deviation_0 * noise_distribution(generator);
		double noise_2_0 = 0.25 * in_standard_deviation_0 * noise_distribution(generator);
		double base_2_0 = base_0 + noise_2_0;
		// double noise_2_1 = 0.4 * in_standard_deviation_1 * noise_distribution(generator);
		double noise_2_1 = 0.25 * in_standard_deviation_1 * noise_distribution(generator);
		double base_2_1 = base_1 + noise_2_1;

		// double noise_3_0 = 0.8 * in_standard_deviation_0 * noise_distribution(generator);
		double noise_3_0 = 0.625 * in_standard_deviation_0 * noise_distribution(generator);
		double base_3_0 = base_0 + noise_3_0;
		// double noise_3_1 = 0.8 * in_standard_deviation_1 * noise_distribution(generator);
		double noise_3_1 = 0.625 * in_standard_deviation_1 * noise_distribution(generator);
		double base_3_1 = base_1 + noise_3_1;

		// double noise_4_0 = 1.6 * in_standard_deviation_0 * noise_distribution(generator);
		double noise_4_0 = 1.5625 * in_standard_deviation_0 * noise_distribution(generator);
		double base_4_0 = base_0 + noise_4_0;
		// double noise_4_1 = 1.6 * in_standard_deviation_1 * noise_distribution(generator);
		double noise_4_1 = 1.5625 * in_standard_deviation_1 * noise_distribution(generator);
		double base_4_1 = base_1 + noise_4_1;

		vector<double> inputs_4{(double)type, base_4_0, base_4_1, 1.5};
		noise_network->activate(inputs_4);
		double positive_noise_4_0 = noise_network->output->acti_vals[0];
		double negative_noise_4_0 = noise_network->output->acti_vals[1];
		double noise_direction_4_0 = noise_network->output->acti_vals[2];
		double positive_noise_4_1 = noise_network->output->acti_vals[3];
		double negative_noise_4_1 = noise_network->output->acti_vals[4];
		double noise_direction_4_1 = noise_network->output->acti_vals[5];
		vector<double> errors_4(6);
		if (noise_4_0 > 0.0) {
			errors_4[0] = noise_4_0 - positive_noise_4_0;
			errors_4[1] = 0.0;
			if (noise_direction_4_0 >= 1.0) {
				errors_4[2] = 0.0;
			} else {
				errors_4[2] = 1.0 - noise_direction_4_0;
			}
		} else {
			errors_4[0] = 0.0;
			errors_4[1] = noise_4_0 - negative_noise_4_0;
			if (noise_direction_4_0 <= -1.0) {
				errors_4[2] = 0.0;
			} else {
				errors_4[2] = -1.0 - noise_direction_4_0;
			}
		}
		if (noise_4_1 > 0.0) {
			errors_4[3] = noise_4_1 - positive_noise_4_1;
			errors_4[4] = 0.0;
			if (noise_direction_4_1 >= 1.0) {
				errors_4[5] = 0.0;
			} else {
				errors_4[5] = 1.0 - noise_direction_4_1;
			}
		} else {
			errors_4[3] = 0.0;
			errors_4[4] = noise_4_1 - negative_noise_4_1;
			if (noise_direction_4_1 <= -1.0) {
				errors_4[5] = 0.0;
			} else {
				errors_4[5] = -1.0 - noise_direction_4_1;
			}
		}
		noise_network->backprop(errors_4);

		{
			vector<double> inputs_3{(double)type, base_3_0, base_3_1, 0.5};
			noise_network->activate(inputs_3);
			double positive_noise_3_0 = noise_network->output->acti_vals[0];
			double negative_noise_3_0 = noise_network->output->acti_vals[1];
			double noise_direction_3_0 = noise_network->output->acti_vals[2];
			double positive_noise_3_1 = noise_network->output->acti_vals[3];
			double negative_noise_3_1 = noise_network->output->acti_vals[4];
			double noise_direction_3_1 = noise_network->output->acti_vals[5];
			vector<double> errors_3(6);
			if (noise_3_0 > 0.0) {
				errors_3[0] = noise_3_0 - positive_noise_3_0;
				errors_3[1] = 0.0;
				if (noise_direction_3_0 >= 1.0) {
					errors_3[2] = 0.0;
				} else {
					errors_3[2] = 1.0 - noise_direction_3_0;
				}
			} else {
				errors_3[0] = 0.0;
				errors_3[1] = noise_3_0 - negative_noise_3_0;
				if (noise_direction_3_0 <= -1.0) {
					errors_3[2] = 0.0;
				} else {
					errors_3[2] = -1.0 - noise_direction_3_0;
				}
			}
			if (noise_3_1 > 0.0) {
				errors_3[3] = noise_3_1 - positive_noise_3_1;
				errors_3[4] = 0.0;
				if (noise_direction_3_1 >= 1.0) {
					errors_3[5] = 0.0;
				} else {
					errors_3[5] = 1.0 - noise_direction_3_1;
				}
			} else {
				errors_3[3] = 0.0;
				errors_3[4] = noise_3_1 - negative_noise_3_1;
				if (noise_direction_3_1 <= -1.0) {
					errors_3[5] = 0.0;
				} else {
					errors_3[5] = -1.0 - noise_direction_3_1;
				}
			}
			noise_network->backprop(errors_3);
		}

		{
			vector<double> inputs_2{(double)type, base_2_0, base_2_1, -0.5};
			noise_network->activate(inputs_2);
			double positive_noise_2_0 = noise_network->output->acti_vals[0];
			double negative_noise_2_0 = noise_network->output->acti_vals[1];
			double noise_direction_2_0 = noise_network->output->acti_vals[2];
			double positive_noise_2_1 = noise_network->output->acti_vals[3];
			double negative_noise_2_1 = noise_network->output->acti_vals[4];
			double noise_direction_2_1 = noise_network->output->acti_vals[5];
			vector<double> errors_2(6);
			if (noise_2_0 > 0.0) {
				errors_2[0] = noise_2_0 - positive_noise_2_0;
				errors_2[1] = 0.0;
				if (noise_direction_2_0 >= 1.0) {
					errors_2[2] = 0.0;
				} else {
					errors_2[2] = 1.0 - noise_direction_2_0;
				}
			} else {
				errors_2[0] = 0.0;
				errors_2[1] = noise_2_0 - negative_noise_2_0;
				if (noise_direction_2_0 <= -1.0) {
					errors_2[2] = 0.0;
				} else {
					errors_2[2] = -1.0 - noise_direction_2_0;
				}
			}
			if (noise_2_1 > 0.0) {
				errors_2[3] = noise_2_1 - positive_noise_2_1;
				errors_2[4] = 0.0;
				if (noise_direction_2_1 >= 1.0) {
					errors_2[5] = 0.0;
				} else {
					errors_2[5] = 1.0 - noise_direction_2_1;
				}
			} else {
				errors_2[3] = 0.0;
				errors_2[4] = noise_2_1 - negative_noise_2_1;
				if (noise_direction_2_1 <= -1.0) {
					errors_2[5] = 0.0;
				} else {
					errors_2[5] = -1.0 - noise_direction_2_1;
				}
			}
			noise_network->backprop(errors_2);
		}

		{
			vector<double> inputs_1{(double)type, base_1_0, base_1_1, -1.5};
			noise_network->activate(inputs_1);
			double positive_noise_1_0 = noise_network->output->acti_vals[0];
			double negative_noise_1_0 = noise_network->output->acti_vals[1];
			double noise_direction_1_0 = noise_network->output->acti_vals[2];
			double positive_noise_1_1 = noise_network->output->acti_vals[3];
			double negative_noise_1_1 = noise_network->output->acti_vals[4];
			double noise_direction_1_1 = noise_network->output->acti_vals[5];
			vector<double> errors_1(6);
			if (noise_1_0 > 0.0) {
				errors_1[0] = noise_1_0 - positive_noise_1_0;
				errors_1[1] = 0.0;
				if (noise_direction_1_0 >= 1.0) {
					errors_1[2] = 0.0;
				} else {
					errors_1[2] = 1.0 - noise_direction_1_0;
				}
			} else {
				errors_1[0] = 0.0;
				errors_1[1] = noise_1_0 - negative_noise_1_0;
				if (noise_direction_1_0 <= -1.0) {
					errors_1[2] = 0.0;
				} else {
					errors_1[2] = -1.0 - noise_direction_1_0;
				}
			}
			if (noise_1_1 > 0.0) {
				errors_1[3] = noise_1_1 - positive_noise_1_1;
				errors_1[4] = 0.0;
				if (noise_direction_1_1 >= 1.0) {
					errors_1[5] = 0.0;
				} else {
					errors_1[5] = 1.0 - noise_direction_1_1;
				}
			} else {
				errors_1[3] = 0.0;
				errors_1[4] = noise_1_1 - negative_noise_1_1;
				if (noise_direction_1_1 <= -1.0) {
					errors_1[5] = 0.0;
				} else {
					errors_1[5] = -1.0 - noise_direction_1_1;
				}
			}
			noise_network->backprop(errors_1);
		}

		if ((iter_index + 1) % 100000 == 0) {
			cout << iter_index << endl;
			cout << endl;
		}
	}

	double in_standard_deviation_0 = sqrt(in_variance_0);
	double in_standard_deviation_1 = sqrt(in_variance_1);

	cout << "in_mean_0: " << in_mean_0 << endl;
	cout << "in_variance_0: " << in_variance_0 << endl;
	cout << "in_standard_deviation_0: " << in_standard_deviation_0 << endl;
	cout << "in_mean_1: " << in_mean_1 << endl;
	cout << "in_variance_1: " << in_variance_1 << endl;
	cout << "in_standard_deviation_1: " << in_standard_deviation_1 << endl;

	normal_distribution<double> init_distribution_0(in_mean_0, 2.0 * in_standard_deviation_0);
	normal_distribution<double> init_distribution_1(in_mean_1, 2.0 * in_standard_deviation_1);
	for (int iter_index = 0; iter_index < 40; iter_index++) {
		cout << iter_index << endl;

		int type = type_distribution(generator);
		cout << "type: " << type << endl;

		double start_0 = init_distribution_0(generator);
		cout << "start_0: " << start_0 << endl;
		double start_1 = init_distribution_1(generator);
		cout << "start_1: " << start_1 << endl;

		vector<double> inputs_4{(double)type, start_0, start_1, 1.5};
		noise_network->activate(inputs_4);
		double positive_noise_4_0 = noise_network->output->acti_vals[0];
		cout << "positive_noise_4_0: " << positive_noise_4_0 << endl;
		double negative_noise_4_0 = noise_network->output->acti_vals[1];
		cout << "negative_noise_4_0: " << negative_noise_4_0 << endl;
		double noise_direction_4_0 = noise_network->output->acti_vals[2];
		cout << "noise_direction_4_0: " << noise_direction_4_0 << endl;
		double positive_noise_4_1 = noise_network->output->acti_vals[3];
		cout << "positive_noise_4_1: " << positive_noise_4_1 << endl;
		double negative_noise_4_1 = noise_network->output->acti_vals[4];
		cout << "negative_noise_4_1: " << negative_noise_4_1 << endl;
		double noise_direction_4_1 = noise_network->output->acti_vals[5];
		cout << "noise_direction_4_1: " << noise_direction_4_1 << endl;

		double predicted_base_3_0;
		if (noise_direction_4_0 >= 1.0) {
			predicted_base_3_0 = start_0 - positive_noise_4_0;
		} else if (noise_direction_4_0 <= -1.0) {
			predicted_base_3_0 = start_0 - negative_noise_4_0;
		} else {
			double rand_val = direction_distribution(generator);
			if (rand_val > noise_direction_4_0) {
				predicted_base_3_0 = start_0 - negative_noise_4_0;
			} else {
				predicted_base_3_0 = start_0 - positive_noise_4_0;
			}
		}
		cout << "predicted_base_3_0: " << predicted_base_3_0 << endl;
		double predicted_base_3_1;
		if (noise_direction_4_1 >= 1.0) {
			predicted_base_3_1 = start_1 - positive_noise_4_1;
		} else if (noise_direction_4_1 <= -1.0) {
			predicted_base_3_1 = start_1 - negative_noise_4_1;
		} else {
			double rand_val = direction_distribution(generator);
			if (rand_val > noise_direction_4_1) {
				predicted_base_3_1 = start_1 - negative_noise_4_1;
			} else {
				predicted_base_3_1 = start_1 - positive_noise_4_1;
			}
		}
		cout << "predicted_base_3_1: " << predicted_base_3_1 << endl;

		vector<double> inputs_3{(double)type, predicted_base_3_0, predicted_base_3_1, 0.5};
		noise_network->activate(inputs_3);
		double positive_noise_3_0 = noise_network->output->acti_vals[0];
		double negative_noise_3_0 = noise_network->output->acti_vals[1];
		double noise_direction_3_0 = noise_network->output->acti_vals[2];
		double positive_noise_3_1 = noise_network->output->acti_vals[3];
		double negative_noise_3_1 = noise_network->output->acti_vals[4];
		double noise_direction_3_1 = noise_network->output->acti_vals[5];

		double predicted_base_2_0;
		if (noise_direction_3_0 >= 1.0) {
			predicted_base_2_0 = predicted_base_3_0 - positive_noise_3_0;
		} else if (noise_direction_3_0 <= -1.0) {
			predicted_base_2_0 = predicted_base_3_0 - negative_noise_3_0;
		} else {
			double rand_val = direction_distribution(generator);
			if (rand_val > noise_direction_3_0) {
				predicted_base_2_0 = predicted_base_3_0 - negative_noise_3_0;
			} else {
				predicted_base_2_0 = predicted_base_3_0 - positive_noise_3_0;
			}
		}
		double predicted_base_2_1;
		if (noise_direction_3_1 >= 1.0) {
			predicted_base_2_1 = predicted_base_3_1 - positive_noise_3_1;
		} else if (noise_direction_3_1 <= -1.0) {
			predicted_base_2_1 = predicted_base_3_1 - negative_noise_3_1;
		} else {
			double rand_val = direction_distribution(generator);
			if (rand_val > noise_direction_3_1) {
				predicted_base_2_1 = predicted_base_3_1 - negative_noise_3_1;
			} else {
				predicted_base_2_1 = predicted_base_3_1 - positive_noise_3_1;
			}
		}

		vector<double> inputs_2{(double)type, predicted_base_2_0, predicted_base_2_1, -0.5};
		noise_network->activate(inputs_2);
		double positive_noise_2_0 = noise_network->output->acti_vals[0];
		double negative_noise_2_0 = noise_network->output->acti_vals[1];
		double noise_direction_2_0 = noise_network->output->acti_vals[2];
		double positive_noise_2_1 = noise_network->output->acti_vals[3];
		double negative_noise_2_1 = noise_network->output->acti_vals[4];
		double noise_direction_2_1 = noise_network->output->acti_vals[5];

		double predicted_base_1_0;
		if (noise_direction_2_0 >= 1.0) {
			predicted_base_1_0 = predicted_base_2_0 - positive_noise_2_0;
		} else if (noise_direction_2_0 <= -1.0) {
			predicted_base_1_0 = predicted_base_2_0 - negative_noise_2_0;
		} else {
			double rand_val = direction_distribution(generator);
			if (rand_val > noise_direction_2_0) {
				predicted_base_1_0 = predicted_base_2_0 - negative_noise_2_0;
			} else {
				predicted_base_1_0 = predicted_base_2_0 - positive_noise_2_0;
			}
		}
		double predicted_base_1_1;
		if (noise_direction_2_1 >= 1.0) {
			predicted_base_1_1 = predicted_base_2_1 - positive_noise_2_1;
		} else if (noise_direction_2_1 <= -1.0) {
			predicted_base_1_1 = predicted_base_2_1 - negative_noise_2_1;
		} else {
			double rand_val = direction_distribution(generator);
			if (rand_val > noise_direction_2_1) {
				predicted_base_1_1 = predicted_base_2_1 - negative_noise_2_1;
			} else {
				predicted_base_1_1 = predicted_base_2_1 - positive_noise_2_1;
			}
		}

		vector<double> inputs_1{(double)type, predicted_base_1_0, predicted_base_1_1, -1.5};
		noise_network->activate(inputs_1);
		double positive_noise_1_0 = noise_network->output->acti_vals[0];
		double negative_noise_1_0 = noise_network->output->acti_vals[1];
		double noise_direction_1_0 = noise_network->output->acti_vals[2];
		double positive_noise_1_1 = noise_network->output->acti_vals[3];
		double negative_noise_1_1 = noise_network->output->acti_vals[4];
		double noise_direction_1_1 = noise_network->output->acti_vals[5];

		double predicted_base_0;
		if (noise_direction_1_0 >= 1.0) {
			predicted_base_0 = predicted_base_1_0 - positive_noise_1_0;
		} else if (noise_direction_1_0 <= -1.0) {
			predicted_base_0 = predicted_base_1_0 - negative_noise_1_0;
		} else {
			double rand_val = direction_distribution(generator);
			if (rand_val > noise_direction_1_0) {
				predicted_base_0 = predicted_base_1_0 - negative_noise_1_0;
			} else {
				predicted_base_0 = predicted_base_1_0 - positive_noise_1_0;
			}
		}
		cout << "predicted_base_0: " << predicted_base_0 << endl;
		double predicted_base_1;
		if (noise_direction_1_1 >= 1.0) {
			predicted_base_1 = predicted_base_1_1 - positive_noise_1_1;
		} else if (noise_direction_1_1 <= -1.0) {
			predicted_base_1 = predicted_base_1_1 - negative_noise_1_1;
		} else {
			double rand_val = direction_distribution(generator);
			if (rand_val > noise_direction_1_1) {
				predicted_base_1 = predicted_base_1_1 - negative_noise_1_1;
			} else {
				predicted_base_1 = predicted_base_1_1 - positive_noise_1_1;
			}
		}
		cout << "predicted_base_1: " << predicted_base_1 << endl;

		cout << endl;
	}

	cout << "Done" << endl;
}
