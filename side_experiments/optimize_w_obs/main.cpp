/**
 * - world model is not about predictions
 *   - but simply about what is located where, and what is moving where
 *   - in humans, built naturally using optical flow/sensor fusion?
 * 
 * - world model enables localization
 *   - helps maintain identity/integrity of long solutions
 *     - breaks down solution into modular segments
 */

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

using namespace std;

int seed;

default_random_engine generator;

// ProblemType* problem_type;
// Solution* solution;
// Solution* solution_duplicate;

int run_index;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	// problem_type = new TypeMinesweeper();



	// delete problem_type;
	// delete solution;

	cout << "Done" << endl;
}
