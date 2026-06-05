#include "branch_node.h"

#include <iostream>

#include "branch_network.h"
#include "constants.h"
#include "experiment.h"
#include "run.h"
#include "utilities.h"
#include "wrapper.h"

using namespace std;

void BranchNode::step(int& action,
					  bool& is_next,
					  Run* run) {
	this->original_network->activate(run->state);
	this->branch_network->activate(run->state);

	bool is_branch;
	if (this->branch_network->output->acti_vals[0] >= this->original_network->output->acti_vals[0]) {
		is_branch = true;
	} else {
		is_branch = false;
	}

	#if defined(MDEBUG) && MDEBUG
	if (run->wrapper->curr_run_seed%2 == 0) {
		is_branch = true;
	} else {
		is_branch = false;
	}
	run->wrapper->curr_run_seed = xorshift(run->wrapper->curr_run_seed);
	#endif /* MDEBUG */

	if (is_branch) {
		run->node_context = this->branch_next_node;
	} else {
		run->node_context = this->original_next_node;
	}
}
