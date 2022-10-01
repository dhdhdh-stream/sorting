#include <chrono>
#include <iostream>
#include <thread>
#include <random>

#include "fold_network.h"
#include "network.h"

using namespace std;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	Network* init_network = new Network(0, 0, 2);
	Network* network = new Network(4, 50, 2);
	Network* combine_network = new Network(3, 20, 1);

	double sum_error = 0.0;
	for (int epoch_index = 1; epoch_index < 50000; epoch_index++) {
		if (epoch_index%100 == 0) {
			cout << endl;
			cout << epoch_index << endl;
			cout << "sum_error: " << sum_error << endl;
			sum_error = 0.0;
		}

		for (int iter_index = 0; iter_index < 100; iter_index++) {
			vector<AbstractNetworkHistory*> network_historys;

			double loop_state[2] = {};

			// behind the loop
			int second_val_on = -1+rand()%2*2;

			vector<double> init_input;
			init_network->activate(init_input);
			loop_state[0] = init_network->output->acti_vals[0];
			loop_state[1] = init_network->output->acti_vals[1];

			int sum = 0;
			for (int i = 0; i < rand()%6; i++) {
				int first_value = rand()%2;
				int second_value = rand()%2;

				sum += first_value;
				if (second_val_on == 1) {
					sum += second_value;
				}

				vector<double> input;
				input.push_back(loop_state[0]);
				input.push_back(loop_state[1]);
				input.push_back(first_value);
				input.push_back(second_value);
				network->activate(input, network_historys);
				loop_state[0] = network->output->acti_vals[0];
				loop_state[1] = network->output->acti_vals[1];
			}

			vector<double> combine_input;
			combine_input.push_back(loop_state[0]);
			combine_input.push_back(loop_state[1]);
			combine_input.push_back(second_val_on);
			combine_network->activate(combine_input);

			vector<double> combine_errors;
			combine_errors.push_back(sum - combine_network->output->acti_vals[0]);
			sum_error += abs(combine_errors[0]);

			if (epoch_index < 40000) {
				combine_network->backprop(combine_errors, 0.01);
			} else {
				combine_network->backprop(combine_errors, 0.001);
			}
			double loop_state_errors[2];
			loop_state_errors[0] = combine_network->input->errors[0];
			combine_network->input->errors[0] = 0.0;
			loop_state_errors[1] = combine_network->input->errors[1];
			combine_network->input->errors[1] = 0.0;

			while (network_historys.size() > 0) {
				network_historys.back()->reset_weights();

				vector<double> errors{loop_state_errors[0], loop_state_errors[1]};
				if (epoch_index < 40000) {
					network->backprop(errors, 0.01);
				} else {
					network->backprop(errors, 0.001);
				}
				loop_state_errors[0] = network->input->errors[0];
				network->input->errors[0] = 0.0;
				loop_state_errors[1] = network->input->errors[1];
				network->input->errors[1] = 0.0;

				delete network_historys.back();
				network_historys.pop_back();
			}

			vector<double> errors{loop_state_errors[0], loop_state_errors[1]};
			if (epoch_index < 40000) {
				init_network->backprop(errors, 0.01);
			} else {
				init_network->backprop(errors, 0.001);
			}
		}
	}

	cout << "Done" << endl;
}
