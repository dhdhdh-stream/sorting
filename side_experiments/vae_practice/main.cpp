// - probably not worth it
//   - unlikely to be extremely reliable or sharp
//     - and any issues send predictions into unknown space
//       - where things are unpredictable anyways
//       - and simulating things out unreliable and also not worth it

// - have to kind of treat things like gradient descent?
//   - stick with small changes
//   - just predict a direction
//     - instead of details + simulation

// TODO: experiment with whether long training needed for edge cases
// - when states are already trained

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "vae.h"

using namespace std;

int seed;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	VAE* vae = new VAE(1, 1, 1);
	// double hidden_1_average_max_update = 0.0;
	// double means_average_max_update = 0.0;
	// double log_vars_average_max_update = 0.0;
	// double hidden_2_average_max_update = 0.0;
	// double output_average_max_update = 0.0;

	uniform_int_distribution<int> input_distribution(0, 1);
	// for (int iter_index = 0; iter_index < 10000000; iter_index++) {
	for (int iter_index = 0; iter_index < 100000000; iter_index++) {
		double val = -0.5 + input_distribution(generator);

		double kl_factor;
		// if (iter_index >= 10000000) {
		// if (iter_index >= 100000000) {
		// 	kl_factor = 1.0;
		if (iter_index >= 10000000) {
			kl_factor = 0.1;
		} else {
			// kl_factor = (double)iter_index / 10000000.0;
			kl_factor = (double)iter_index / 100000000.0;
		}

		// double kl_factor;
		// // int remainder = iter_index % 20000000;
		// int remainder = iter_index % 200000000;
		// // if (remainder > 10000000) {
		// if (remainder > 100000000) {
		// 	// kl_factor = 2.0 - (double)remainder / 10000000.0;
		// 	kl_factor = 2.0 - (double)remainder / 100000000.0;
		// } else {
		// 	// kl_factor = (double)remainder / 10000000.0;
		// 	kl_factor = (double)remainder / 100000000.0;
		// }

		vector<double> inputs{(double)val};
		vae->activate(inputs);

		vector<double> errors{val - vae->output->acti_vals[0]};

		vae->backprop(errors,
					  kl_factor);
		// vae->init_backprop(errors,
		// 				   hidden_1_average_max_update,
		// 				   means_average_max_update,
		// 				   log_vars_average_max_update,
		// 				   hidden_2_average_max_update,
		// 				   output_average_max_update);

		if ((iter_index + 1) % 100000 == 0) {
			cout << iter_index << endl;
			cout << "val: " << val << endl;
			cout << "vae->means->acti_vals[0]: " << vae->means->acti_vals[0] << endl;
			cout << "vae->log_vars->acti_vals[0]: " << vae->log_vars->acti_vals[0] << endl;
			double variance = exp(vae->log_vars->acti_vals[0]);
			cout << "variance: " << variance << endl;
			cout << "vae->rand_vals[0]: " << vae->rand_vals[0] << endl;
			cout << "vae->decoder_input->acti_vals[0]: " << vae->decoder_input->acti_vals[0] << endl;
			cout << "vae->output->acti_vals[0]: " << vae->output->acti_vals[0] << endl;
			cout << "kl_factor: " << kl_factor << endl;
			cout << endl;
		}
	}

	cout << "Done" << endl;
}
