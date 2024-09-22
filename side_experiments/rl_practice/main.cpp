// TODO:
// - use softmax/cross entropy to allow variation
// - use A3C to transform predicted vs. actual scores into whether performed action was target action

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "rl_practice.h"

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

	problem_type = new TypeRLPractice();



	delete problem_type;

	cout << "Done" << endl;
}
