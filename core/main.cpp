#include <chrono>
#include <iostream>
#include <thread>
#include <random>

#include "action_dictionary.h"
#include "solution.h"

using namespace std;

default_random_engine generator;

Solution* solution;
ActionDictionary* action_dictionary;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	solution = new Solution();

	cout << "Done" << endl;
}
