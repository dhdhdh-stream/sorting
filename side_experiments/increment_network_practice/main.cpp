#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

using namespace std;

int seed;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;



	cout << "Done" << endl;
}
