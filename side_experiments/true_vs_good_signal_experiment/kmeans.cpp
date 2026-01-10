// - actually, not looking for kmeans, but looking to split above 0.0 and below 0.0
// - also, don't use kmeans, but its generalization: gaussian mixture models
//   - but maybe pin mean at 0

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

void kmeans(vector<double>& points,
			vector<double>& means) {
	double max_val = points[0];
	double min_val = points[0];
	for (int i_index = 1; i_index < (int)points.size(); i_index++) {
		if (points[i_index] > max_val) {
			max_val = points[i_index];
		} else if (points[i_index] < min_val) {
			min_val = points[i_index];
		}
	}

	means = vector<double>(3);
	double diff = max_val - min_val;
	means[0] = min_val + 0.25 * diff;
	means[1] = min_val + 0.5 * diff;
	means[2] = min_val + 0.75 * diff;

	vector<int> assignment(points.size());
	for (int i_index = 0; i_index < (int)points.size(); i_index++) {
		double best_distance = abs(means[0] - points[i_index]);
		int best_index = 0;
		for (int m_index = 1; m_index < 3; m_index++) {
			double curr_distance = abs(means[m_index] - points[i_index]);
			if (curr_distance < best_distance) {
				best_distance = curr_distance;
				best_index = m_index;
			}
		}

		assignment[i_index] = best_index;
	}

	while (true) {
		for (int m_index = 0; m_index < 3; m_index++) {
			double sum_vals = 0.0;
			int count = 0;
			for (int i_index = 0; i_index < (int)points.size(); i_index++) {
				if (assignment[i_index] == m_index) {
					sum_vals += points[i_index];
					count++;
				}
			}

			means[m_index] = sum_vals / (double)count;
		}

		bool assignment_changed = false;
		for (int i_index = 0; i_index < (int)points.size(); i_index++) {
			double best_distance = abs(means[0] - points[i_index]);
			int best_index = 0;
			for (int m_index = 1; m_index < 3; m_index++) {
				double curr_distance = abs(means[m_index] - points[i_index]);
				if (curr_distance < best_distance) {
					best_distance = curr_distance;
					best_index = m_index;
				}
			}

			if (best_index != assignment[i_index]) {
				assignment_changed = true;

				assignment[i_index] = best_index;
			}
		}

		if (!assignment_changed) {
			break;
		}
	}
}

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	// vector<double> points{0.0, 0.1, 0.1, 0.2, 0.4, 0.5, 0.5, 0.6, 0.8, 0.9, 0.9, 1.0};
	vector<double> points{0.1, 0.1, 0.1, 0.1, 0.4, 0.5, 0.5, 0.6};

	vector<double> means;
	kmeans(points,
		   means);

	for (int m_index = 0; m_index < (int)means.size(); m_index++) {
		cout << m_index << ": " << means[m_index] << endl;
	}

	cout << "Done" << endl;
}
