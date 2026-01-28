// - if don't want separate signal actions
//   - can try explicitly including nodes as pre vs. post

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "logic_helpers.h"
#include "logic_tree.h"
#include "logic_wrapper.h"
#include "minesweeper_flag.h"
// #include "moving_signal.h"
// #include "moving_vs_opening.h"
// #include "multi_sum.h"
// #include "multiplicative.h"
// #include "offset_w_diff.h"
// #include "spot_diff.h"

using namespace std;

int seed;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	AbstractProblem* problem = new MinesweeperFlag();
	// AbstractProblem* problem = new MovingSignal();
	// AbstractProblem* problem = new MovingVsOpening();
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
		int starting_node_counter = logic_wrapper->logic_tree->node_counter;

		vector<double> obs;
		double target_val;
		problem->get_train_instance(obs,
									target_val);

		logic_experiment_helper(obs,
								target_val,
								logic_wrapper,
								problem);

		if (logic_wrapper->logic_tree->node_counter != starting_node_counter) {
			logic_wrapper->save("saves/", filename);
		}
	}

	delete problem;
	delete logic_wrapper;

	cout << "Done" << endl;
}
