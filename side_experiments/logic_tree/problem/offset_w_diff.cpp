#include "offset_w_diff.h"

#include "globals.h"

using namespace std;

void OffsetWDiff::get_instance(vector<double>& obs,
							   double& target_val) {
	uniform_int_distribution<int> obs_distribution(-10, 10);

	vector<double> before;
	for (int i_index = 0; i_index < 10; i_index++) {
		before.push_back(obs_distribution(generator));
	}

	vector<double> after = before;

	uniform_int_distribution<int> diff_distribution(5, 9);
	int diff = diff_distribution(generator);
	while (true) {
		after[diff] = obs_distribution(generator);
		if (before[diff] != after[diff]) {
			break;
		}
	}

	uniform_int_distribution<int> offset_distribution(0, 5);
	int offset = offset_distribution(generator);
	for (int o_index = 0; o_index < offset; o_index++) {
		after.erase(after.begin());
		after.push_back(obs_distribution(generator));
	}

	obs.insert(obs.end(), before.begin(), before.end());
	obs.insert(obs.end(), after.begin(), after.end());

	target_val = diff;
}
