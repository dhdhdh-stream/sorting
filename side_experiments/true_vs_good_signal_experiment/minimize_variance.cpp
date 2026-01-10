// - can separately try above 0 and below 0

#include <algorithm>
#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "network.h"

using namespace std;

int seed;

default_random_engine generator;

void find_split(vector<double>& points,
				double& split) {
	sort(points.begin(), points.end());

	// temp
	cout << "points:";
	for (int i_index = 0; i_index < (int)points.size(); i_index++) {
		cout << " " << points[i_index];
	}
	cout << endl;

	double best_sum_variance = numeric_limits<double>::max();
	double best_zero_variance;
	double best_top_val_average;
	double best_top_variance;
	for (int i_index = 1; i_index < (int)points.size(); i_index++) {
		double zero_sum_variance = 0.0;
		for (int zero_index = 0; zero_index < i_index; zero_index++) {
			zero_sum_variance += points[zero_index] * points[zero_index];
		}

		double top_sum_vals = 0.0;
		for (int top_index = i_index; top_index < (int)points.size(); top_index++) {
			top_sum_vals += points[top_index];
		}
		double top_val_average = top_sum_vals / ((double)points.size() - i_index);

		double top_sum_variance = 0.0;
		for (int top_index = i_index; top_index < (int)points.size(); top_index++) {
			top_sum_variance += (points[top_index] - top_val_average) * (points[top_index] - top_val_average);
		}

		double sum_variance = zero_sum_variance + top_sum_variance;

		if (sum_variance < best_sum_variance) {
			best_sum_variance = sum_variance;

			double zero_variance = zero_sum_variance / (double)i_index;
			best_zero_variance = zero_variance;

			best_top_val_average = top_val_average;

			double top_variance = top_sum_variance / ((double)points.size() - i_index);
			best_top_variance = top_variance;
		}
	}

	double zero_standard_deviation = sqrt(best_zero_variance);
	double top_standard_deviation = sqrt(best_top_variance);

	cout << "best_top_val_average: " << best_top_val_average << endl;
	cout << "zero_standard_deviation: " << zero_standard_deviation << endl;
	cout << "top_standard_deviation: " << top_standard_deviation << endl;

	split = best_top_val_average * zero_standard_deviation / (zero_standard_deviation + top_standard_deviation);
}

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	vector<double> points{0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.7, 0.9, 0.9, 1.0, 1.0, 1.0, 1.0, 1.1, 1.1, 1.3};
	// vector<double> points{0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 1.18, 1.18, 1.19, 1.2, 1.2, 1.2, 1.2, 1.2, 1.2, 1.21, 1.22, 1.22};

	double split;
	find_split(points,
			   split);

	cout << "split: " << split << endl;

	cout << "Done" << endl;
}
