#include <chrono>
#include <iostream>
#include <thread>
#include <random>

using namespace std;

default_random_engine generator;
double global_sum_error;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;



	cout << "Done" << endl;
}