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
	
	explore.setup_cycle();

	for (int i = 1; i < 2000000; i++) {
		if (i%1000000 == 0) {
			cout << "tune " << i << endl;
		}
		explore.solution->iteration(true, false);
	}
	for (int i = 1; i < 20000000; i++) {
		if (i%1000000 == 0) {
			cout << "explore " << i << endl;
		}
		explore.solution->iteration(false, false);
	}
	explore.save();

	cout << "Done" << endl;
}
