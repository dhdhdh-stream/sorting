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

void calc_top(vector<double>& points,
			  double& split) {
	double max_val = points[0];
	for (int i_index = 1; i_index < (int)points.size(); i_index++) {
		if (points[i_index] > max_val) {
			max_val = points[i_index];
		}
	}

	double zero_variance = 1.0;
	double top_mean = max_val;
	double top_variance = 1.0;

	while (true) {
		vector<double> zero_likelihood(points.size());
		vector<double> top_likelihood(points.size());
		for (int i_index = 0; i_index < (int)points.size(); i_index++) {
			if (points[i_index] == top_mean) {
				zero_likelihood[i_index] = 0.0;
				top_likelihood[i_index] = 1.0;
			} else {
				zero_likelihood[i_index] = zero_variance / (points[i_index] * points[i_index]);

				double top_distance = top_mean - points[i_index];
				top_likelihood[i_index] = top_variance / (top_distance * top_distance);

				double sum_likelihood = zero_likelihood[i_index] + top_likelihood[i_index];
				zero_likelihood[i_index] /= sum_likelihood;
				top_likelihood[i_index] /= sum_likelihood;

				/**
				 * - not sure if numerically stable
				 *   - but can't be too bad since this would be equivalent to K-means for these datapoints?
				 */
				if (points[i_index] > top_mean) {
					zero_likelihood[i_index] = 0.0;
					top_likelihood[i_index] = 1.0;
				}
			}
		}

		double sum_top_vals = 0.0;
		double sum_top_weight = 0.0;
		for (int i_index = 0; i_index < (int)points.size(); i_index++) {
			sum_top_vals += top_likelihood[i_index] * points[i_index];
			sum_top_weight += top_likelihood[i_index];
		}

		double new_top_mean = sum_top_vals / sum_top_weight;
		if (new_top_mean == top_mean) {
			// temp
			for (int i_index = 0; i_index < (int)points.size(); i_index++) {
				cout << "points[i_index]: " << points[i_index] << endl;
				cout << "zero_likelihood[i_index]: " << zero_likelihood[i_index] << endl;
				cout << "top_likelihood[i_index]: " << top_likelihood[i_index] << endl;
			}

			break;
		}

		top_mean = new_top_mean;

		double sum_zero_variance = 0.0;
		double sum_zero_weight = 0.0;
		double sum_top_variance = 0.0;
		for (int i_index = 0; i_index < (int)points.size(); i_index++) {
			sum_zero_variance += zero_likelihood[i_index] * points[i_index] * points[i_index];
			sum_zero_weight += zero_likelihood[i_index];

			double top_distance = top_mean - points[i_index];
			sum_top_variance += top_likelihood[i_index] * top_distance * top_distance;
		}

		zero_variance = sum_zero_variance / sum_zero_weight;
		top_variance = sum_top_variance / sum_top_weight;
	}

	double zero_standard_deviation = sqrt(zero_variance);
	double top_standard_deviation = sqrt(top_variance);

	cout << "top_mean: " << top_mean << endl;
	cout << "zero_standard_deviation: " << zero_standard_deviation << endl;
	cout << "top_standard_deviation: " << top_standard_deviation << endl;

	split = top_mean * zero_standard_deviation / (zero_standard_deviation + top_standard_deviation);
}

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	// vector<double> points{0.1, 0.1, 0.1, 0.1, 0.4, 0.5, 0.5, 0.6};
	// vector<double> points{0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.7, 0.9, 0.9, 1.0, 1.0, 1.0, 1.0, 1.1, 1.1, 1.3};
	vector<double> points{0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 1.18, 1.18, 1.19, 1.2, 1.2, 1.2, 1.2, 1.2, 1.2, 1.21, 1.22, 1.22};

	double split;
	calc_top(points,
			 split);

	cout << "split: " << split << endl;

	cout << "Done" << endl;
}
