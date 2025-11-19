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
// #include "moving_signal.h"
// #include "multi_sum.h"
// #include "multiplicative.h"
#include "offset_w_diff.h"
// #include "spot_diff.h"

// TODO: try explicitly looking for equals
// TODO: obs data is sequential
// - can try transformers?
//   - though would mean crunching lots of data all the time
// - obs data being sequential does give meaning to looking for equals

// - in the general case, there will be motion, but not linear motion like for transformers
//   - the positional encoding will not work
//     - then there's no point

// - try adding to value instead of replacing
//   - does mean many networks per run

using namespace std;

int seed;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	// AbstractProblem* problem = new MovingSignal();
	// AbstractProblem* problem = new MultiSum();
	// AbstractProblem* problem = new Multiplicative();
	AbstractProblem* problem = new OffsetWDiff();
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
		problem->get_instance(obs,
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
