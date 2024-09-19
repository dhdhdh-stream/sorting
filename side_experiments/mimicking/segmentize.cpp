#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "globals.h"
#include "minesweeper.h"
#include "problem.h"
#include "sample.h"
#include "segment_helpers.h"

using namespace std;

int seed;

default_random_engine generator;

ProblemType* problem_type;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	problem_type = new TypeMinesweeper();

	vector<Sample*> samples;
	vector<vector<int>> starting_indexes;
	vector<vector<int>> ending_indexes;
	for (int sample_index = 0; sample_index < 200; sample_index++) {
		Sample* sample = new Sample(sample_index);
		samples.push_back(sample);

		vector<int> curr_starting_indexes{0};
		vector<int> curr_ending_indexes{(int)sample->locations.size()-1};

		while (true) {
			find_max_split(sample,
						   curr_starting_indexes,
						   curr_ending_indexes);

			double distance = calc_distance(sample,
											curr_starting_indexes,
											curr_ending_indexes);

			double normalized_distance = distance / sample->locations.size();
			if (normalized_distance < 1.0) {
				starting_indexes.push_back(curr_starting_indexes);
				ending_indexes.push_back(curr_ending_indexes);
				break;
			}
		}
	}

	for (int sample_index = 0; sample_index < (int)samples.size(); sample_index++) {
		delete samples[sample_index];
	}

	delete problem_type;

	cout << "Done" << endl;
}