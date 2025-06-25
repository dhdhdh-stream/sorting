#include "solution_helpers.h"

#include <iostream>

using namespace std;

const double MAX_DISTANCE = 1.05;
const double MAX_EXTREME_RATIO = 0.05;

bool is_match(vector<double>& t_scores) {
	if (t_scores.size() == 0) {
		return true;
	}

	double sum_distance = 0.0;
	int num_normal = 0;
	int num_extreme = 0;
	for (int t_index = 0; t_index < (int)t_scores.size(); t_index++) {
		double distance = abs(t_scores[t_index]);
		if (distance > 3.0) {
			num_extreme++;
		} else {
			sum_distance += distance;
			num_normal++;
		}
	}

	if (num_normal == 0) {
		return false;
	}

	double average_distance = sum_distance / (double)num_normal;
	double extreme_ratio = num_extreme / (double)t_scores.size();
	cout << "average_distance: " << average_distance << endl;
	cout << "extreme_ratio: " << extreme_ratio << endl;
	if (average_distance > MAX_DISTANCE
			|| extreme_ratio > MAX_EXTREME_RATIO) {
		return false;
	} else {
		return true;
	}
}
