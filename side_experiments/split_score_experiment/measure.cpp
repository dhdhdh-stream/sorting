#include <algorithm>
#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "simpler.h"

using namespace std;

int seed;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	ProblemType* problem_type = new TypeSimpler();

	uniform_int_distribution<int> random_action_distribution(0, problem_type->num_possible_actions()-1);

	/**
	 * - average
	 */
	vector<double> existing_vals;
	for (int iter_index = 0; iter_index < 1000; iter_index++) {
		Problem* problem = problem_type->get_problem();

		double target_val = problem->score_result();
		existing_vals.push_back(target_val);

		delete problem;
	}
	double sum_vals = 0.0;
	for (int h_index = 0; h_index < (int)existing_vals.size(); h_index++) {
		sum_vals += existing_vals[h_index];
	}
	double val_average = sum_vals / (double)existing_vals.size();
	cout << "val_average: " << val_average << endl;
	double sum_variance = 0.0;
	for (int h_index = 0; h_index < (int)existing_vals.size(); h_index++) {
		sum_variance += (existing_vals[h_index] - val_average)
			* (existing_vals[h_index] - val_average);
	}
	double val_standard_deviation = sqrt(sum_variance / (double)existing_vals.size());
	cout << "val_standard_deviation: " << val_standard_deviation << endl;

	cout << "Done" << endl;
}
