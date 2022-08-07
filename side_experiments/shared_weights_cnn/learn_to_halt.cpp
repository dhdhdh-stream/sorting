#include <chrono>
#include <iostream>
#include <limits>
#include <thread>
#include <random>

#include "action.h"
#include "collapse_network.h"
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

	ifstream learn_sort_save_file;
	learn_sort_save_file.open("learn_sort_save.txt");
	CollapseNetwork learn_sort(learn_sort_save_file);
	learn_sort_save_file.close();

	Network halt_network(4, 100, 4);

	double sum_error = 0.0;
	for (int epoch_index = 1; epoch_index < 50000; epoch_index++) {

		if (epoch_index%1000 == 0) {
			cout << endl;
			cout << "epoch_index: " << epoch_index << endl;
			cout << "sum_error: " << sum_error << endl;
			sum_error = 0.0;
		}

		for (int iter_index = 0; iter_index < 100; iter_index++) {
			Problem p;
			int iterations = 3 + rand()%5;

			if (epoch_index%1000 == 0 && iter_index == 0) {
				p.print();
				cout << "iterations: " << iterations << endl;
			}

			vector<double> global_vals;

			vector<NetworkHistory*> network_historys;

			double state[3] = {};
			vector<vector<double>> time_vals;
			for (int i = 0; i < iterations; i++) {
				double observation = p.get_observation();

				vector<double> inputs;
				inputs.push_back(state[0]);
				inputs.push_back(state[1]);
				inputs.push_back(state[2]);
				inputs.push_back(observation);
				halt_network.activate(inputs, network_historys);

				if (epoch_index%1000 == 0 && iter_index == 0) {
					cout << i << ": " << halt_network.val_val->acti_vals[3] << endl;
				}

				state[0] = halt_network.val_val->acti_vals[0];
				state[1] = halt_network.val_val->acti_vals[1];
				state[2] = halt_network.val_val->acti_vals[2];

				vector<double> time_val;
				time_val.push_back(observation);
				time_vals.push_back(time_val);

				p.perform_action(a);
			}

			double score = p.score_result();

			vector<double> fan_results = learn_sort.fan(iterations,
														time_vals,
														global_vals);

			if (epoch_index%1000 == 0 && iter_index == 0) {
				for (int i = 0; i < iterations; i++) {
					cout << "fan " << i << ": " << fan_results[i] << endl;
				}
			}

			double state_errors[3] = {};
			for (int i = iterations-1; i >= 0; i--) {
				network_historys[i]->reset_weights();
				
				vector<double> errors;
				errors.push_back(state_errors[0]);
				errors.push_back(state_errors[1]);
				errors.push_back(state_errors[2]);

				double target;
				if (score == 1.0) {
					if (fan_results[i] > 1.0) {
						target = 0.0;
					} else {
						target = abs(1.0 - fan_results[i]);
					}
				} else {
					if (fan_results[i] < 0.0) {
						target = 0.0;
					} else {
						target = abs(0.0 - fan_results[i]);
					}
				}
				double error;
				if (target == 0.0 && halt_network.val_val->acti_vals[3] < 0.0) {
					error = 0.0;
				} else {
					error = target - halt_network.val_val->acti_vals[3];
				}
				errors.push_back(error);
				sum_error += abs(error);

				halt_network.backprop(errors);

				for (int s_index = 0; s_index < 3; s_index++) {
					state_errors[s_index] = halt_network.input->errors[s_index];
					halt_network.input->errors[s_index] = 0.0;
				}
			}

			for (int i = 0; i < iterations; i++) {
				delete network_historys[i];
			}
		}

		double max_update = 0.0;
		halt_network.calc_max_update(max_update,
									 0.001,
									 0.2);
		double factor = 1.0;
		if (max_update > 0.01) {
			factor = 0.01/max_update;
		}
		halt_network.update_weights(factor,
									0.001,
									0.2);
	}

	ofstream learn_halt_save;
	learn_halt_save.open("learn_halt_save.txt");
	halt_network.save(learn_halt_save);
	learn_halt_save.close();

	cout << "Done" << endl;
}