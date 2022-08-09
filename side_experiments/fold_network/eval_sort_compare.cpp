#include <chrono>
#include <iostream>
#include <limits>
#include <thread>
#include <random>

#include "action.h"
#include "fold_network.h"
#include "network.h"
#include "problem.h"
#include "utilities.h"

using namespace std;

default_random_engine generator;

const Action a(0.0, RIGHT);

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	Network network(4, 40, 3);

	double sum_error = 0.0;
	for (int epoch_index = 1; epoch_index < 100000; epoch_index++) {

		if (epoch_index%1000 == 0) {
			cout << endl;
			cout << epoch_index << endl;
			cout << "sum_error: " << sum_error << endl;
			sum_error = 0.0;
		}

		for (int iter_index = 0; iter_index < 100; iter_index++) {
			Problem p;
			int iterations = (int)p.initial_world.size()-1;

			if (epoch_index%1000 == 0 && iter_index == 0) {
				p.print();
			}

			vector<double> global_vals;
			global_vals.push_back(p.get_observation());
			p.perform_action(a);

			vector<NetworkHistory*> network_historys;

			double state[2] = {};
			for (int iter_index = 0; iter_index < iterations; iter_index++) {
				double observation = p.get_observation();

				vector<double> inputs;
				inputs.push_back(state[0]);
				inputs.push_back(state[1]);
				inputs.push_back(global_vals[0]);
				inputs.push_back(observation);

				network.activate(inputs, network_historys);

				state[0] = network.val_val->acti_vals[0];
				state[1] = network.val_val->acti_vals[1];

				p.perform_action(a);
			}

			double result = network.val_val->acti_vals[2];
			if (epoch_index%1000 == 0 && iter_index == 0) {
				cout << "result: " << result << endl;
			}

			double target = p.score_result();
			
			double state_errors[2] = {};

			// top iteration
			network_historys[iterations-1]->reset_weights();
			vector<double> errors;
			errors.push_back(state_errors[0]);	// 0.0
			errors.push_back(state_errors[1]);	// 0.0
			if (target == 1.0) {
				if (result < 1.0) {
					errors.push_back(target - result);
					sum_error += abs(target - result);
				} else {
					errors.push_back(0.0);
				}
			} else {
				if (result > 0.0) {
					errors.push_back(target - result);
					sum_error += abs(target - result);
				} else {
					errors.push_back(0.0);
				}
			}
			network.backprop(errors);
			state_errors[0] = network.input->errors[0];
			network.input->errors[0] = 0.0;
			state_errors[1] = network.input->errors[1];
			network.input->errors[1] = 0.0;

			for (int i = iterations-2; i >= 0; i--) {
				network_historys[i]->reset_weights();
				vector<double> errors;
				errors.push_back(state_errors[0]);
				errors.push_back(state_errors[1]);
				errors.push_back(0.0);
				network.backprop(errors);
				state_errors[0] = network.input->errors[0];
				network.input->errors[0] = 0.0;
				state_errors[1] = network.input->errors[1];
				network.input->errors[1] = 0.0;
			}

			for (int i = 0; i < iterations; i++) {
				delete network_historys[i];
			}
		}

		double max_update = 0.0;
		network.calc_max_update(max_update,
								0.001,
								0.2);
		double factor = 1.0;
		if (max_update > 0.01) {
			factor = 0.01/max_update;
		}
		network.update_weights(factor,
							   0.001,
							   0.2);
	}

	cout << "Done" << endl;
}
