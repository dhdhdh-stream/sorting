#include "moving_signal.h"

#include "globals.h"

using namespace std;

const int WORLD_SIZE = 5;

void MovingSignal::get_train_instance(vector<double>& obs,
									  double& target_val) {
	uniform_int_distribution<int> location_distribution(0, WORLD_SIZE-1);
	int signal_x = location_distribution(generator);
	int signal_y = location_distribution(generator);

	uniform_int_distribution<int> obs_distribution(-10, 10);
	for (int x_index = 0; x_index < WORLD_SIZE; x_index++) {
		for (int y_index = 0; y_index < WORLD_SIZE; y_index++) {
			if (signal_x == x_index + 1
					&& signal_y == y_index) {
				obs.push_back(20.0);
			} else if (signal_x == x_index - 1
					&& signal_y == y_index) {
				obs.push_back(20.0);
			} else if (signal_x == x_index
					&& signal_y == y_index + 1) {
				obs.push_back(20.0);
			} else if (signal_x == x_index
					&& signal_y == y_index - 1) {
				obs.push_back(20.0);
			} else {
				obs.push_back(obs_distribution(generator));
			}
		}
	}

	uniform_int_distribution<int> noise_distribution(-5, 5);
	target_val = obs[signal_x * WORLD_SIZE + signal_y] + noise_distribution(generator);
}

void MovingSignal::get_test_instance(vector<double>& obs,
									 double& target_val) {
	get_train_instance(obs,
					   target_val);
}
