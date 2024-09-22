// - samples have too much variance to be easily summarized?
//   - variation short term, medium term, and long term

// - maybe learn slowly, sample by sample?
// - start performing a sample in its entirety
//   - learn branches, but can only branch a limited distance
//     - to prevent large sections of sample from being deleted
//   - or maybe can quit if certain conditions apply?

// - logic is an efficient way to share information
//   - can share complex decision making through sharing simple rules that have the right complex consequences
//   - "if you're in situation a and b, do x" maybe means:
//     - look for spots that are a in solution, add branch checking for condition b, then do x
//   - but not relevant for exploring/building solutions(?)
//     - decision making can already be broken down

// - maybe swap between sequential mode and Markov mode?
//   - and don't need state (assume fully captured by context)
//     - only use obs/world model? (which for Minesweeper captures all state anyways)
//   - then use Markov mode to evaluate samples
//     - if sample did something surprising, then that's what should be evaluated deeper?

// - Markov mode might also improve generalization, meaning need to explore less?
//   - or at least, explore only what's surprising

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "globals.h"
#include "minesweeper.h"
#include "problem.h"
#include "sample.h"
#include "sample_graph.h"
#include "sample_graph_helpers.h"

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

	SampleGraph* graph = new SampleGraph();

	Sample* initial_sample = new Sample(0);
	graph->init(initial_sample);
	delete initial_sample;

	// for (int sample_index = 1; sample_index < 200; sample_index++) {
	for (int sample_index = 1; sample_index < 50; sample_index++) {
		Sample* sample = new Sample(sample_index);
		add_sample(graph,
				   sample);
		delete sample;
	}

	ofstream output_file;
	output_file.open("display.txt");
	graph->save_for_display(output_file);
	output_file.close();

	delete graph;

	delete problem_type;

	cout << "Done" << endl;
}
