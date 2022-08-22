#include <chrono>
#include <iostream>
#include <thread>
#include <random>

#include "explore.h"

using namespace std;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	Explore explore;

	cout << "Done" << endl;
}
