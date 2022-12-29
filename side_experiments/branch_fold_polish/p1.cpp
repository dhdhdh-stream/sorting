#include <chrono>
#include <iostream>
#include <thread>
#include <mutex>
#include <random>

using namespace std;

default_random_engine generator;
double global_sum_error;

int id_counter;
mutex id_counter_mtx;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;



	cout << "Done" << endl;
}