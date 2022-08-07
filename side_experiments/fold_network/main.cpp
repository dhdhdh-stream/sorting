#include <chrono>
#include <iostream>
#include <limits>
#include <thread>
#include <random>

#include "action.h"
#include "fold_network.h"
#include "utilities.h"

using namespace std;

default_random_engine generator;

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
			int iterations = 2 + rand()%5;

			vector<double> problem;
			double max = numeric_limits<double>::lowest();
			for (int i = 0; i < iterations; i++) {
				double rand_value = randnorm()*20.0 - 10.0;
				problem.push_back(rand_value);

				if (rand_value > max) {
					max = rand_value;
				}
			}

			if (epoch_index%1000 == 0 && iter_index == 0) {
				cout << "problem:";
				for (int i = 0; i < iterations; i++) {
					cout << " " << problem[i];
				}
				cout << endl;
			}

			vector<double> global_vals;

			vector<vector<double>> time_vals;
			for (int iter_index = 0; iter_index < iterations; iter_index++) {
				double observation = problem[iter_index];

				vector<double> time_val;
				time_val.push_back(observation);
				time_val.push_back(observation);
				time_val.push_back(observation/2.0);
				time_val.push_back(randnorm());
				time_val.push_back(randnorm());
				time_vals.push_back(time_val);
			}

			network.train(iterations,
						  time_vals,
						  global_vals);

			double result = network.output->acti_vals[0];
			if (epoch_index%1000 == 0 && iter_index == 0) {
				cout << "result: " << result << endl;
			}

			double target = max;
			vector<double> errors;
			errors.push_back(target - result);
			sum_error += abs(target - result);
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

	FoldNetwork network(5, 6, 0, 1);

	one_pass(network, "flat", 20000);
	network.next_step();

	one_pass(network, "fold #0", 10000);
	network.next_step();

	one_pass(network, "fold #1", 10000);
	network.next_step();

	one_pass(network, "fold #2", 10000);
	network.next_step();

	one_pass(network, "fold #3", 10000);
	network.next_step();

	one_pass(network, "fold #4", 10000);
	network.next_step();

	one_pass(network, "fold #5", 10000);
	network.next_step();

	one_pass(network, "clean", 10000);

	cout << "Done" << endl;
}
