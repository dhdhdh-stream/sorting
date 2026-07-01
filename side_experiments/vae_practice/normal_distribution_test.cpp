#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "network.h"

using namespace std;

int seed;

default_random_engine generator;

const int NUM_SAMPLES = 1000;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	uniform_int_distribution<int> distribution(0, 9);
	vector<double> samples(NUM_SAMPLES);
	for (int s_index = 0; s_index < NUM_SAMPLES; s_index++) {
		if (distribution(generator) == 0) {
			samples[s_index] = 1.0;
		} else {
			samples[s_index] = -1.0;
		}
	}

	double sum_vals = 0.0;
	for (int s_index = 0; s_index < NUM_SAMPLES; s_index++) {
		sum_vals += samples[s_index];
	}
	double mean = sum_vals / NUM_SAMPLES;
	double sum_variance = 0.0;
	for (int s_index = 0; s_index < NUM_SAMPLES; s_index++) {
		sum_variance += (samples[s_index] - mean) * (samples[s_index] - mean);
	}
	double variance = sqrt(sum_variance / NUM_SAMPLES);

	cout << "mean: " << mean << endl;
	cout << "variance: " << variance << endl;

	cout << "Done" << endl;
}
