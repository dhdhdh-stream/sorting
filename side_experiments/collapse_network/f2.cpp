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

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	vector<int> time_sizes;
	CollapseNetwork network(time_sizes, 2);

	Action a(0.0, RIGHT);

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
			if (epoch_index%1000 == 0 && iter_index == 0) {
				p.print();
			}

			vector<double> global_vals;
			global_vals.push_back(p.get_observation());
			p.perform_action(a);
			global_vals.push_back(p.get_observation());
			p.perform_action(a);

			vector<vector<double>> time_vals;

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

	cout << "Done" << endl;
}
