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

	// ifstream explore_save_file;
	// explore_save_file.open("../saves/1661285450.txt");
	// Explore explore(explore_save_file);
	// explore_save_file.close();

	Explore explore;
	
	for (int cycle_index = 0; cycle_index < 5; cycle_index++) {
		explore.setup_cycle();

		for (int i = 1; i < 5000000; i++) {
			if (i%1000000 == 0) {
				cout << "tune " << i << endl;
				explore.solution->iteration(true, true);
			} else {
				explore.solution->iteration(true, false);
			}
		}
		for (int i = 1; i < 50000000; i++) {
			if (i%1000000 == 0) {
				cout << "explore " << i << endl;
				explore.solution->iteration(false, true);
			} else {
				explore.solution->iteration(false, false);
			}
		}

		explore.cleanup_cycle();

		explore.save();
	}

	cout << "Done" << endl;
}
