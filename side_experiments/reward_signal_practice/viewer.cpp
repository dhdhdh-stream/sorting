#include <iostream>

#include "globals.h"
#include "pattern.h"

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

	ifstream input_file;
	input_file.open("saves/main.txt");
	Pattern* pattern = new Pattern(input_file);
	input_file.close();

	cout << "actions:";
	for (int a_index = 0; a_index < (int)pattern->actions.size(); a_index++) {
		cout << " " << pattern->actions[a_index];
	}
	cout << endl;

	cout << "keypoints:" << endl;
	for (int k_index = 0; k_index < (int)pattern->keypoints.size(); k_index++) {
		cout << pattern->keypoints[k_index] << " " << pattern->keypoint_standard_deviations[k_index] << endl;
	}

	cout << "Done" << endl;
}
