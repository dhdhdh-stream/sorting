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

// - have to use signal alongside other
//   - as only see a window and not the full picture
//   - so also don't worry about signal being fundamental
//     - it will only be a snapshot of a particular solution

// - use history to drive progress
//   - don't use explore, as will too likely just reinforce existing
// - but using history will likely lead to valuing side effects?
//   - whereas using explore likely avoids valuing side effects
// - signal can also have been maxed out
//   - maybe just as simple as if after a couple iterations, if signal did not lead to improvement, ignore
//     - either can never find something that improves upon signal
//     - or signal just gives false positives

// - multiply signal by consistency
//   - between 0 and 1
//     - (but can go below 0 and above 1)

// - just see if anything positive can be found
//   - either signal works and make note of it and use it
//     - or give up on signal for that scope

// - what happens in A3C?
//   - for bad signals, will chase and more samples will be gathered, will regress back to the mean, and become irrelevant
//   - for good signals, will occasionally not chase, reinforcing strength of signal
//     - for me, random explore is so bad, that signal is always likely to be reinforced
//       - A3C has same issue though, it's just everything is so bad that not as dramatic

// - equivalent of A3C is just explore
//   - likely leads to preserving existing
//     - doesn't drive innovation

// - try more focused experiments?

// - what represents progress in Minesweeper?
//   - opening up more correctly
//     - opening up more without seeing error
//   - flagging more correctly
//     - to understand whether a flag is correct or not, likely need to have performed correct flag so many times that it must have been added?
//       - but perhaps true only early
//         - maybe does generalize later on

// TODO: try distinguishing between moving vs. opening
// TODO: try also changing minesweeper score function to make good signals easier?

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
