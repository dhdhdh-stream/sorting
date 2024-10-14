// - can be difficult to take path like sample
//   - need to know both that the path can be taken and need right follow up
//     - like, if someone else uses the 8 corners strategy, not meaningful to incorporate into layer strategy

// - need to start by learning someone's strategy to the best of ability

// - find efficient way to summarize samples
//   - same decision making, paying attention to the same things

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "globals.h"
#include "minesweeper.h"
#include "sample.h"
#include "scope.h"
#include "solution.h"

using namespace std;

const int NUM_ALIGNMENT_TRIES = 100;

int seed;

default_random_engine generator;

ProblemType* problem_type;
Solution* solution;

int run_index;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	problem_type = new TypeMinesweeper();

	solution = new Solution();
	solution->load("", "main");

	Sample* sample = new Sample("", 0);

	double best_score = numeric_limits<double>::max();
	Alignment best_alignment(sample);
	for (int t_index = 0; t_index < NUM_ALIGNMENT_TRIES; t_index++) {
		Alignment curr_alignment(sample);
		vector<ContextLayer> context;
		solution->scopes[0]->align_activate(
			curr_alignment,
			context);

		double curr_score = curr_alignment.score();
		if (curr_score < best_score) {
			best_score = curr_score;
			best_alignment = curr_alignment;
		}
	}

	best_alignment.print();

	delete sample;

	delete problem_type;
	delete solution;

	cout << "Done" << endl;
}
