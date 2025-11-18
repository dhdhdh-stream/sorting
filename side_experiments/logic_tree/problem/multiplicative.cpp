#include "multiplicative.h"

#include "globals.h"

using namespace std;

void Multiplicative::get_instance(vector<double>& obs,
								  double& target_val) {
	uniform_int_distribution<int> base_distribution(1, 5);
	uniform_int_distribution<int> val_distribution(0, 25);
	int base = base_distribution(generator);
	obs.push_back(base);
	int val = val_distribution(generator);
	obs.push_back(val);

	uniform_int_distribution<int> noise_distribution(-5, 5);
	if (val % base == 0) {
		target_val = 10.0 + noise_distribution(generator);
	} else {
		target_val = -10.0 + noise_distribution(generator);
	}
}
