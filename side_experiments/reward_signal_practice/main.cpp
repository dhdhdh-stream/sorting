#include <iostream>

#include "globals.h"
#include "network.h"
#include "pattern.h"
#include "simple.h"
#include "solution_helpers.h"

using namespace std;

int seed;

default_random_engine generator;

ProblemType* problem_type;

const int NUM_TRIES = 100;

const int PATTERN_LENGTH = 10;

const int NUM_KEYPOINTS = 5;
const int NUM_INPUTS = 5;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	problem_type = new TypeSimple();

	Pattern* best_pattern = NULL;
	double best_average_misguess = 0.0;

	uniform_int_distribution<int> pre_distribution(0, 50);
	uniform_int_distribution<int> action_distribution(0, 1);
	for (int t_index = 0; t_index < NUM_TRIES; t_index++) {
		vector<int> full_sequence;

		int pattern_start_index = pre_distribution(generator);
		for (int p_index = 0; p_index < pattern_start_index; p_index++) {
			full_sequence.push_back(action_distribution(generator));
		}

		vector<int> actions;
		for (int a_index = 0; a_index < PATTERN_LENGTH; a_index++) {
			actions.push_back(action_distribution(generator));
		}
		full_sequence.insert(full_sequence.end(), actions.begin(), actions.end());

		vector<int> keypoints;
		for (int i_index = 0; i_index < PATTERN_LENGTH; i_index++) {
			keypoints.push_back(i_index);
		}

		vector<int> inputs;
		for (int i_index = 0; i_index < NUM_INPUTS; i_index++) {
			uniform_int_distribution<int> distribution(0, (int)keypoints.size()-1);
			int random_index = distribution(generator);
			inputs.push_back(keypoints[random_index]);
			keypoints.erase(keypoints.begin() + random_index);
		}

		vector<double> keypoint_averages;
		vector<double> keypoint_standard_deviations;
		measure_keypoints(full_sequence,
						  pattern_start_index,
						  keypoints,
						  keypoint_averages,
						  keypoint_standard_deviations);

		Network* network;
		double average_misguess;
		double misguess_standard_deviation;
		train_pattern_network(actions,
							  keypoints,
							  keypoint_averages,
							  keypoint_standard_deviations,
							  inputs,
							  network,
							  average_misguess,
							  misguess_standard_deviation);

		cout << "average_misguess: " << average_misguess << endl;
		cout << "misguess_standard_deviation: " << misguess_standard_deviation << endl;

		if (best_pattern == NULL) {
			best_pattern = new Pattern(actions,
									   keypoints,
									   keypoint_averages,
									   keypoint_standard_deviations,
									   inputs,
									   network);
		} else if (average_misguess < best_average_misguess) {
			delete best_pattern;

			best_pattern = new Pattern(actions,
									   keypoints,
									   keypoint_averages,
									   keypoint_standard_deviations,
									   inputs,
									   network);
		} else {
			delete network;
		}
	}

	ofstream output_file;
	output_file.open("saves/main.txt");
	best_pattern->save(output_file);
	output_file.close();

	cout << "Done" << endl;
}
