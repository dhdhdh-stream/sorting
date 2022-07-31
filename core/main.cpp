#include <iostream>

#include "solver.h"

using namespace std;

int SEED;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	SEED = (unsigned)time(NULL);
	srand(SEED);
	cout << "Seed: " << SEED << endl;

	Solver s;
	s.run();

	cout << "Done" << endl;
}
