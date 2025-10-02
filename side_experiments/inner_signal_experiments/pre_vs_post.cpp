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

	Network* pre_network = new Network(1);
	Network* post_network = new Network(2);

	uniform_int_distribution<int> add_distribution(0, 4);

	for (int iter_index = 0; iter_index < 300000; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << "iter_index: " << iter_index << endl;
		}

		vector<vector<double>> pre_inputs;
		vector<vector<double>> post_inputs;

		double val = 0.0;
		for (int s_index = 0; s_index < NUM_STEPS; s_index++) {
			vector<double> curr_post_inputs;

			pre_inputs.push_back(vector<double>{val});
			curr_post_inputs.push_back(val);

			val += add_distribution(generator);

			curr_post_inputs.push_back(val);

			post_inputs.push_back(curr_post_inputs);
		}

		for (int s_index = 0; s_index < NUM_STEPS; s_index++) {
			{
				pre_network->activate(pre_inputs[s_index]);

				double error = val - pre_network->output->acti_vals[0];

				pre_network->backprop(error);
			}

			{
				post_network->activate(post_inputs[s_index]);

				double error = val - post_network->output->acti_vals[0];

				post_network->backprop(error);
			}
		}
	}

	for (int iter_index = 0; iter_index < 10; iter_index++) {
		double val = 0.0;
		for (int s_index = 0; s_index < NUM_STEPS; s_index++) {
			vector<double> curr_post_inputs;

			double start_val = val;
			vector<double> curr_pre_inputs{val};
			curr_post_inputs.push_back(val);

			val += add_distribution(generator);

			double end_val = val;
			curr_post_inputs.push_back(val);

			pre_network->activate(curr_pre_inputs);
			post_network->activate(curr_post_inputs);

			double diff = post_network->output->acti_vals[0] - pre_network->output->acti_vals[0];

			cout << "start_val: " << start_val << endl;
			cout << "end_val: " << end_val << endl;
			cout << "pre_network->output->acti_vals[0]: " << pre_network->output->acti_vals[0] << endl;
			cout << "post_network->output->acti_vals[0]: " << post_network->output->acti_vals[0] << endl;
			cout << "diff: " << diff << endl;
			cout << endl;
		}
	}

	cout << "Done" << endl;
}
