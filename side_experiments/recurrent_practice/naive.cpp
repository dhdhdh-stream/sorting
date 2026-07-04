// average: 0.15425
// misguess_average: 0.130457

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "constants.h"
#include "network.h"

using namespace std;

int seed;

default_random_engine generator;

// const int NUM_STATES = 4;
const int NUM_STATES = 2;
// const int NUM_STATES = 1;

const int NUM_SAMPLES = 4000;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	vector<vector<int>> contexts;
	vector<vector<int>> actions;
	vector<double> target_vals;

	geometric_distribution<int> num_actions_distribution(0.1);
	uniform_int_distribution<int> action_distribution(0, 3);
	uniform_int_distribution<int> context_distribution(0, 4);
	for (int i_index = 0; i_index < NUM_SAMPLES; i_index++) {
		int num_actions = num_actions_distribution(generator);
		
		vector<int> curr_contexts;
		vector<int> curr_actions;
		for (int a_index = 0; a_index < num_actions; a_index++) {
			if (context_distribution(generator) == 0) {
				curr_contexts.push_back(1);
			} else {
				curr_contexts.push_back(0);
			}
			curr_actions.push_back(action_distribution(generator));
		}
		contexts.push_back(curr_contexts);
		actions.push_back(curr_actions);

		int sum_distance = 0;
		for (int a_index = 0; a_index < (int)curr_contexts.size(); a_index++) {
			switch (curr_actions[a_index]) {
			case 0:
				if (curr_contexts[a_index] == 1) {
					sum_distance++;
				} else {
					sum_distance--;
				}
				break;
			case 1:
				if (curr_contexts[a_index] == 1) {
					sum_distance--;
				} else {
					sum_distance++;
				}
				break;
			}
		}
		if (sum_distance == 1) {
			target_vals.push_back(1.0);
		} else {
			target_vals.push_back(0.0);
		}
	}

	double sum_target_vals = 0.0;
	for (int h_index = 0; h_index < NUM_SAMPLES; h_index++) {
		sum_target_vals += target_vals[h_index];
	}
	double average = sum_target_vals / NUM_SAMPLES;
	cout << "average: " << average << endl;

	{
		double sum_misguess = 0.0;
		for (int h_index = 0; h_index < NUM_SAMPLES; h_index++) {
			sum_misguess += (target_vals[h_index] - average)
				* (target_vals[h_index] - average);
		}
		double misguess_average = sum_misguess / (double)NUM_SAMPLES;
		cout << "misguess_average: " << misguess_average << endl;
	}

	cout << "Done" << endl;
}
