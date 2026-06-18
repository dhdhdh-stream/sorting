#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "minesweeper.h"
#include "wrapper.h"

using namespace std;

int seed;

default_random_engine generator;

int main(int argc, char* argv[]) {
	string filename;
	if (argc > 1) {
		filename = argv[1];
	} else {
		filename = "main.txt";
	}

	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	ProblemType* problem_type = new TypeMinesweeper();

	Wrapper* wrapper = new Wrapper(problem_type);

	wrapper->save("saves/", filename);

	delete wrapper;

	cout << "Done" << endl;
}
