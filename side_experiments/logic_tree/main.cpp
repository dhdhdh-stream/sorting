// - if don't want separate signal actions
//   - can try explicitly including nodes as pre vs. post

// - BTW, for scopes, context can be useful to pass in as an input
//   - number of actions
//   - number of times scope has already appeared

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "logic_helpers.h"
#include "logic_tree.h"
#include "logic_wrapper.h"
#include "minesweeper_signal.h"
// #include "moving_signal.h"
// #include "multi_sum.h"
// #include "multiplicative.h"
// #include "offset_w_diff.h"
// #include "spot_diff.h"

// - try adding to value instead of replacing
//   - does mean many networks per run

// - random explore too crazy
//   - too difficult to draw conclusions
// - using existing solution can lead to poor signals
//   - where signal based on side effect of solution
//     - rather than what's fundamental to problem
// - maybe try filtering?
//   - costs so much effort, but maybe leads to better progress later?
//     - (and is what a human would consider "understanding" a problem)

// - maybe use samples from different runs?

using namespace std;

int seed;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	AbstractProblem* problem = new MinesweeperSignal();
	// AbstractProblem* problem = new MovingSignal();
	// AbstractProblem* problem = new MultiSum();
	// AbstractProblem* problem = new Multiplicative();
	// AbstractProblem* problem = new OffsetWDiff();
	// AbstractProblem* problem = new SpotDiff();

	string filename;
	LogicWrapper* logic_wrapper;
	if (argc > 1) {
		filename = argv[1];
		logic_wrapper = new LogicWrapper(
			"saves/",
			filename);
	} else {
		filename = "main.txt";
		logic_wrapper = new LogicWrapper(problem);
	}

	while (true) {
		auto starting_num_nodes = logic_wrapper->logic_tree->nodes.size();

		vector<double> obs;
		double target_val;
		problem->get_train_instance(obs,
									target_val);

		logic_experiment_helper(logic_wrapper->logic_tree->root,
								obs,
								target_val,
								logic_wrapper);

		logic_wrapper->update(problem);

		if (logic_wrapper->logic_tree->nodes.size() != starting_num_nodes) {
			logic_wrapper->save("saves/", filename);
		}
	}

	delete problem;
	delete logic_wrapper;

	cout << "Done" << endl;
}
