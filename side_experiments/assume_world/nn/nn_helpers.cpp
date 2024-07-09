#include "nn_helpers.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "network.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_ITERS = 30;
#else
const int TRAIN_ITERS = 300000;
#endif /* MDEBUG */

void train_network(vector<vector<vector<double>>>& inputs,
				   vector<double>& target_vals,
				   Network* network) {
	uniform_int_distribution<int> distribution(0, inputs.size()-1);
	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		int rand_index = distribution(generator);

		network->activate(inputs[rand_index]);

		double error = target_vals[rand_index] - network->output->acti_vals[0];

		network->backprop(error);
	}
}
