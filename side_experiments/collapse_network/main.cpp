#include <chrono>
#include <iostream>
#include <thread>
#include <random>

#include "action.h"
#include "collapse_network.h"
#include "problem.h"
#include "utilities.h"

using namespace std;

default_random_engine generator;

const Action a(0.0, RIGHT);

void one_pass(CollapseNetwork& network,
			  string pass_name,
			  int iterations) {
	double sum_error = 0.0;
	for (int epoch_index = 1; epoch_index < iterations; epoch_index++) {

		if (epoch_index%1000 == 0) {
			cout << endl;
			cout << pass_name << ": " << epoch_index << endl;
			cout << "sum_error: " << sum_error << endl;
			sum_error = 0.0;
		}

		for (int iter_index = 0; iter_index < 100; iter_index++) {
			Problem p;
			if (epoch_index%1000 == 0 && iter_index == 0) {
				p.print();
			}

			vector<double> global_vals;
			global_vals.push_back(p.get_observation());
			p.perform_action(a);
			global_vals.push_back(p.get_observation());
			p.perform_action(a);

			int iterations = 1+rand()%4;
			if (epoch_index%1000 == 0 && iter_index == 0) {
				cout << "iterations: " << iterations << endl;
			}

			vector<vector<double>> time_vals;
			for (int iter_index = 0; iter_index < iterations; iter_index++) {
				double observation = p.get_observation();

				vector<double> time_val;
				time_val.push_back(observation);
				time_val.push_back(observation);
				time_val.push_back(observation/2.0);
				time_val.push_back(randnorm());
				time_val.push_back(randnorm());
				time_vals.push_back(time_val);
				
				p.perform_action(a);
			}

			network.activate(time_vals, global_vals);

			double result = network.output->acti_vals[0];
			if (epoch_index%1000 == 0 && iter_index == 0) {
				cout << "result: " << result << endl;
			}

			double target = p.score_result();
			vector<double> errors;
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
		}

		for (int i = 0; i < 4; i++) {
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
	}
}

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	vector<int> time_sizes;
	time_sizes.push_back(5);
	time_sizes.push_back(5);
	time_sizes.push_back(5);
	time_sizes.push_back(5);
	CollapseNetwork network(time_sizes, 2);

	one_pass(network, "flat", 100000);
	network.next_step(1);

	one_pass(network, "collapse #0", 20000);
	network.next_step(1);

	one_pass(network, "collapse #1", 20000);
	network.next_step(1);

	one_pass(network, "collapse #2", 20000);
	network.next_step(1);

	one_pass(network, "collapse #3", 20000);
	
	CollapseNetwork network_copy_1(&network);
	network_copy_1.next_step(1);
	one_pass(network_copy_1, "fold #0 w/ 1", 40000);

	CollapseNetwork network_copy_2(&network);
	network_copy_2.next_step(2);
	one_pass(network_copy_2, "fold #0 w/ 2", 40000);

	CollapseNetwork network_copy_3(&network);
	network_copy_3.next_step(3);
	one_pass(network_copy_3, "fold #0 w/ 3", 40000);

	cout << "Done" << endl;
}
