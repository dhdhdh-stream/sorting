#include <chrono>
#include <iostream>
#include <thread>
#include <random>

#include "solver.h"

using namespace std;

default_random_engine generator;

Solver* solver;
int counter;
std::mutex counter_mtx;
// chrono::time_point<chrono::steady_clock> start;

void run_solver() {
	while (true) {
		counter_mtx.lock();
		int current_counter = counter;
		counter++;
		counter_mtx.unlock();

		if (current_counter%500000 == 0) {
			solver->single_pass(true);

			// chrono::time_point<chrono::steady_clock> end = chrono::steady_clock::now();
			// cout << "Duration: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << endl;
			// start = end;
		} else {
			solver->single_pass(false);
		}

		if (current_counter%20000000 == 0) {
			solver->save();
		}
	}
}

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	solver = new Solver();
	counter = 1;
	// start = chrono::steady_clock::now();

	run_solver();

	// std::thread threads[10];

	// for (int i = 0; i < 10; i++) {
	// 	threads[i] = thread(run_solver);
	// }

	// for (auto& th : threads) {
	// 	th.join();
	// }

	delete solver;

	cout << "Done" << endl;
}
