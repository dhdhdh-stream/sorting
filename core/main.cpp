#include <chrono>
#include <iostream>
#include <thread>
#include <random>

#include "action_dictionary.h"
#include "solution.h"

using namespace std;

default_random_engine generator;

long int id;
Solution* solution;
ActionDictionary* action_dictionary;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	// ifstream save_file;
	// save_file.open("../saves/1663120144.txt");
	// solution = new Solution(save_file);
	// save_file.close();

	solution = new Solution();

	for (int i = 0; i < 2000000; i++) {
		solution->iteration(true, false);
	}

	for (int i = 0; i < 2000000000; i++) {
		if (i%1000000 == 0) {
			cout << i << endl;
		}
		solution->iteration(false, false);
	}

	cout << "Done" << endl;
}
