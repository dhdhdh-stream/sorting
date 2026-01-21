// TODO: try noise summing up networks over time

// TODO: try XOR

// - if already tunneling without thinking about global, then go full reinforcement learning?
//   - train from out to in?
// TODO: test what happens when different parts have different information
// - actually, can't go reinforcement learning?
//   - can't train one spot's limited view using another spot's limited view
//   - have to attribute
//     - and then don't use attribute as truth, but just as tunnel
// - one spot can have multiple networks
//   - predicting against different layers

// - possible but difficult to get 1%
// - very very difficult to get 0.1%

// - high epoch size is key to reducing noise

// - optimize for 1%

// - seems like not really a risk of overtraining

// - epoch size = 1000, train iters = 1000000:
//   - 101.679
//   - 50.6216
//   - 99.7546
//   - 106.42
//   - 93.2611
//   - 91.7298
//   - 161.908
//   - 90.0622
//   - 79.0943
//   - 109.615
//   - 244.168
//   - 128.637
//   - 78.7252
//   - 147.393
//   - 141.044
//   - 86.9846
//   - 116.972
//   - 64.7905
//   - 140.169
//   - 84.2313
// - too few iters to converge

// - epoch size = 1000, train iters = 300000:
//   - 372.196
//   - 488.962
//   - 308.518
//   - 292.419
//   - 482.487
//   - 376.59
//   - 491.396
//   - 198.195
//   - 387.654
//   - 435.496
//   - 136.149
//   - 502.099
//   - 522.185
//   - 497.345
//   - 382.929
//   - 525.36
//   - 529.593
//   - 460.024
//   - 543.348
//   - 393.957

// - epoch size = 200, train iters = 1000000:
//   - 91.0279
//   - 78.4144
//   - 96.1605
//   - 143.748
//   - 111.625
//   - 133.983
//   - 69.1037
//   - 103.357
//   - 146.66
//   - 64.6645
//   - 187.182
//   - 73.8377
//   - 94.1446
//   - 175.726
//   - 103.998
//   - 147.051
//   - 129.565
//   - 153.018
//   - 108.592
//   - 89.203

// - epoch size = 20, train iters = 1000000:
//   - 173.145
//   - 128.556
//   - 246.01
//   - 281.902
//   - 126.064
//   - 350.964
//   - 134.437
//   - 87.4001
//   - 171.504
//   - 171.673
//   - 136.216
//   - 123.414
//   - 281.363
//   - 518.411
//   - 124.685
//   - 191.152
//   - 90.9612
//   - 131.751
//   - 128.735
//   - 136.339

// - epoch size = 20, train iters = 300000:
//   - 254.486
//   - 330.974
//   - 272.95
//   - 378.807
//   - 342.102
//   - 254.697
//   - 153.503
//   - 417.566
//   - 273.912
//   - 216.672
//   - 353.795
//   - 268.054
//   - 203.323
//   - 305.046
//   - 295.903
//   - 260.529
//   - 217.986
//   - 347.795
//   - 252.88
//   - 526.365

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "network.h"

using namespace std;

int seed;

default_random_engine generator;

const int NUM_INPUTS = 20;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	uniform_int_distribution<int> input_distribution(-1, 1);
	uniform_int_distribution<int> noise_distribution(-100, 100);

	for (int try_index = 0; try_index < 20; try_index++) {
		Network* network = new Network(NUM_INPUTS);

		for (int iter_index = 0; iter_index < 300000; iter_index++) {
		// for (int iter_index = 0; iter_index < 1000000; iter_index++) {
		// for (int iter_index = 0; iter_index < 10000000; iter_index++) {
			vector<double> inputs(NUM_INPUTS);
			for (int i_index = 0; i_index < NUM_INPUTS; i_index++) {
				inputs[i_index] = input_distribution(generator);
			}
			double noise = noise_distribution(generator);

			double result = inputs[0] + noise;

			network->activate(inputs);

			double error = result - network->output->acti_vals[0];

			network->backprop(error);

			if (iter_index % 100000 == 0) {
				cout << iter_index << endl;
			}
		}

		double sum_misguess = 0.0;
		for (int i_index = 0; i_index < 1000; i_index++) {
			vector<double> inputs(NUM_INPUTS);
			for (int i_index = 0; i_index < NUM_INPUTS; i_index++) {
				inputs[i_index] = input_distribution(generator);
			}

			network->activate(inputs);

			sum_misguess += (inputs[0] - network->output->acti_vals[0]) * (inputs[0] - network->output->acti_vals[0]);
		}
		cout << "sum_misguess: " << sum_misguess << endl;

		delete network;
	}

	cout << "Done" << endl;
}
