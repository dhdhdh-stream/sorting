// - try using softmax, and normalize error signals going in?

// - so obs networks goes into discrete
//   - so is this probabilistic?
// - then another network updates state

// - if can be trained using true, then easy to get training data for predict
//   - training data is just true

// - output has to be probabilistic
//   - instead of continuous values, will be probabilities

// - maybe softmax the error?
//   - translate error into discrete

// - or sum the errors, and that tells you how things should change?

// - just use standard softmax backprop
//   - but for softmax output, don't use probabilities, but quash to 0 or 1

// - practice translating errors into probability

// - not direction of error, but error size

// - maybe is predicting error size?
// - so not even softmax, just straight up sum

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "constants.h"
#include "network.h"

using namespace std;

int seed;

default_random_engine generator;

const double MIN_RATIO = 0.0001;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	vector<Network*> networks;
	networks.push_back(new Network(0));
	networks.push_back(new Network(0));

	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		vector<double> predicted(networks.size());
		vector<double> input;
		for (int n_index = 0; n_index < (int)networks.size(); n_index++) {
			networks[n_index]->activate(input);
			predicted[n_index] = networks[n_index]->output->acti_vals[0];
			if (predicted[n_index] < 0.0) {
				predicted[n_index] = 0.0;
			}
		}

		double sum_predicted = 0.0;
		for (int n_index = 0; n_index < (int)networks.size(); n_index++) {
			sum_predicted += predicted[n_index];
		}
		if (sum_predicted < MIN_RATIO) {
			sum_predicted = MIN_RATIO;
		}

		vector<double> ratios(networks.size());
		for (int n_index = 0; n_index < (int)networks.size(); n_index++) {
			ratios[n_index] = 1.0 - predicted[n_index] / sum_predicted;
			if (ratios[n_index] < MIN_RATIO) {
				ratios[n_index] = MIN_RATIO;
			}
		}

		double sum_ratios = 0.0;
		for (int n_index = 0; n_index < (int)networks.size(); n_index++) {
			sum_ratios += ratios[n_index];
		}

		uniform_real_distribution<double> distribution(0.0, sum_ratios);
		double rand_val = distribution(generator);
		int selected_index;
		for (int n_index = 0; n_index < (int)networks.size(); n_index++) {
			rand_val -= ratios[n_index];
			if (rand_val <= 0.0) {
				selected_index = n_index;
				break;
			}
		}

		double target_error;
		if (selected_index == 0) {
			target_error = 0.7;
		} else {
			target_error = 0.3;
		}

		double error = target_error - networks[selected_index]->output->acti_vals[0];
		networks[selected_index]->backprop(error);

		if (iter_index % 1000 == 0) {
			cout << iter_index << endl;
			cout << "predicted[0]: " << predicted[0] << endl;
			cout << "predicted[1]: " << predicted[1] << endl;
			cout << "ratios[0]: " << ratios[0] << endl;
			cout << "ratios[1]: " << ratios[1] << endl;
			cout << "selected_index: " << selected_index << endl;
			cout << endl;
		}
	}

	cout << "Done" << endl;
}
