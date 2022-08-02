#include <iostream>
#include <random>

#include "solver.h"

using namespace std;

default_random_engine generator;

mutex mtx;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	mtx.lock();

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	Solver s;

	for (int i = 0; i < 500000000; i++) {
		if (i%500000 == 0) {
			s.single_pass(true);
		} else {
			s.single_pass(false);
		}
	}

	mtx.unlock();

	cout << "Done" << endl;
}
