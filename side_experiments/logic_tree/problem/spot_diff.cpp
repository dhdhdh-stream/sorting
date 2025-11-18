#include "spot_diff.h"

#include "globals.h"

using namespace std;

const int WORLD_SIZE = 25;

void SpotDiff::get_instance(vector<double>& obs,
							double& target_val) {
	uniform_int_distribution<int> obs_distribution(-10, 10);

	vector<double> original;
	for (int i_index = 0; i_index < WORLD_SIZE; i_index++) {
		original.push_back(obs_distribution(generator));
	}

	uniform_int_distribution<int> diff_distribution(0, 24);
	int diff = diff_distribution(generator);

	vector<double> copy = original;
	copy[diff] = obs_distribution(generator);

	obs.insert(obs.end(), original.begin(), original.end());
	obs.insert(obs.end(), copy.begin(), copy.end());

	target_val = diff;
}
