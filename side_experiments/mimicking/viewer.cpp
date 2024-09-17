#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "globals.h"
#include "minesweeper.h"
#include "problem.h"
#include "sample.h"

using namespace std;

int seed;

default_random_engine generator;

ProblemType* problem_type;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	problem_type = new TypeMinesweeper();

	Sample sample(0);

	for (int s_index = 0; s_index < (int)sample.actions.size(); s_index++) {
		cout << "s_index: " << s_index << endl;

		cout << "actions: " << sample.actions[s_index].move << endl;

		cout << "obs:" << endl;
		for (int x_index = 0; x_index < 5; x_index++) {
			for (int y_index = 0; y_index < 5; y_index++) {
				cout << sample.obs[s_index][5 * y_index + x_index] << " ";
			}
			cout << endl;
		}

		cout << endl;
	}

	delete problem_type;

	cout << "Done" << endl;
}
