#include <chrono>
#include <iostream>
#include <limits>
#include <thread>
#include <random>

#include "action.h"
#include "fold_network.h"
#include "problem.h"
#include "utilities.h"

using namespace std;

default_random_engine generator;

const Action a(0.0, RIGHT);

void one_pass(FoldNetwork& network,
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
			int iterations = (int)p.initial_world.size();

			if (epoch_index%1000 == 0 && iter_index == 0) {
				p.print();
			}

			vector<double> global_vals;

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

			network.train(iterations,
						  time_vals,
						  global_vals);

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

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	FoldNetwork network(5, 6, 0, 2);

	one_pass(network, "flat", 30000);
	network.next_step();
	
	one_pass(network, "fold #0", 20000);
	network.next_step();

	one_pass(network, "fold #1", 20000);
	network.next_step();

	one_pass(network, "fold #2", 20000);
	network.next_step();

	one_pass(network, "fold #3", 20000);
	network.next_step();

	one_pass(network, "fold #4", 20000);
	network.next_step();

	one_pass(network, "fold #5", 20000);
	network.next_step();

	one_pass(network, "clean", 10000);

	cout << "Done" << endl;
}
