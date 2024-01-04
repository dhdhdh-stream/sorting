#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "solution.h"

using namespace std;

default_random_engine generator;

Solution* solution;
string path = "";

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	solution = new Solution();
	solution->init();

	solution->save("main");

	// clear tries
	ofstream output_file;
	output_file.open("saves/main/tries.txt");
	output_file.close();

	delete solution;

	cout << "Done" << endl;
}
