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

	// int seed = (unsigned)time(NULL);
	int seed = 1663018402;
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	solution = new Solution();

	// for (int i = 0; i < 200000; i++) {
	for (int i = 0; i < 2000; i++) {
		solution->iteration(true, false);
	}

	cout << "average_score: " << solution->average_score << endl;
	cout << "start_scope->average_misguess: " << solution->start_scope->average_misguess << endl;
	cout << "start_scope->explore_weight: " << solution->start_scope->explore_weight << endl;

	// for (int i = 0; i < 2000000000; i++) {
	for (int i = 0; i < 20000; i++) {
		if (i%1000000 == 0) {
			cout << i << endl;
		}
		solution->iteration(false, false);
	}

	cout << "Done" << endl;
}
