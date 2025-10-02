// TODO: check if need pre-predict for signals to have any meaning
// TODO: check learning score network against final vs. diff

// - should not predict final?
//   - should predict diff?

// - start is crucial

// - goal should be to get rid of predicting average as much as possible
//   - goal should be to predict exactly diff

// - but pre alone only deals with previous
//   - doesn't include after
//     - is that an issue? or does it get averaged out?
//       - don't have to worry about after
//         - after isn't included in post, but pre is

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "network.h"

using namespace std;

default_random_engine generator;

const int NUM_STEPS = 10;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	Network* neutral_network = new Network(1);

	uniform_int_distribution<int> explore_impact_distribution(-3, 1);
	uniform_int_distribution<int> add_distribution(0, 4);

	for (int iter_index = 0; iter_index < 300000; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << "iter_index: " << iter_index << endl;
		}

		vector<vector<double>> inputs;

		double val = 0.0;
		for (int s_index = 0; s_index < NUM_STEPS; s_index++) {
			val += add_distribution(generator);

			inputs.push_back(vector<double>{val});
		}

		for (int s_index = 0; s_index < NUM_STEPS; s_index++) {
			neutral_network->activate(inputs[s_index]);

			double error = val - neutral_network->output->acti_vals[0];

			neutral_network->backprop(error);
		}
	}

	Network* explore_network = new Network(1);

	for (int iter_index = 0; iter_index < 300000; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << "iter_index: " << iter_index << endl;
		}

		vector<vector<double>> inputs;

		double val = explore_impact_distribution(generator);
		for (int s_index = 0; s_index < NUM_STEPS; s_index++) {
			val += add_distribution(generator);

			inputs.push_back(vector<double>{val});
		}

		for (int s_index = 0; s_index < NUM_STEPS; s_index++) {
			explore_network->activate(inputs[s_index]);

			double error = val - explore_network->output->acti_vals[0];

			explore_network->backprop(error);
		}
	}

	for (int iter_index = 0; iter_index < 10; iter_index++) {
		double explore_impact = explore_impact_distribution(generator);

		double val = 0.0;
		for (int s_index = 0; s_index < NUM_STEPS; s_index++) {
			val += add_distribution(generator);

			vector<double> neutral_input{val};
			neutral_network->activate(neutral_input);
			vector<double> explore_input{val + explore_impact};
			explore_network->activate(explore_input);

			cout << "val: " << val << endl;
			cout << "explore_impact: " << explore_impact << endl;
			cout << "neutral_network->output->acti_vals[0]: " << neutral_network->output->acti_vals[0] << endl;
			cout << "explore_network->output->acti_vals[0]: " << explore_network->output->acti_vals[0] << endl;
			cout << endl;
		}
	}

	cout << "Done" << endl;
}
