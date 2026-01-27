#include "multi_sum.h"

#include "globals.h"

using namespace std;

void MultiSum::get_train_instance(vector<double>& obs,
								  double& target_val) {
	uniform_int_distribution<int> sum_distribution(-5, 5);
	int true_sum = sum_distribution(generator);

	int actual_sum;
	uniform_int_distribution<int> match_distribution(0, 1);
	if (match_distribution(generator) == 0) {
		actual_sum = true_sum;
	} else {
		actual_sum = sum_distribution(generator);
	}

	uniform_int_distribution<int> input_distribution(-3, 3);
	int val1 = input_distribution(generator);
	int val2 = input_distribution(generator);
	int val3 = actual_sum - val1 - val2;

	obs.push_back(true_sum);
	obs.push_back(val1);
	obs.push_back(val2);
	obs.push_back(val3);

	uniform_int_distribution<int> noise_distribution(-5, 5);
	if (actual_sum == true_sum) {
		target_val = 10.0 + noise_distribution(generator);
	} else {
		target_val = -10.0 + noise_distribution(generator);
	}
}

void MultiSum::get_test_instance(vector<double>& obs,
								 double& target_val) {
	get_train_instance(obs,
					   target_val);
}
