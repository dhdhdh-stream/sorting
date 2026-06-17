#include "branch_node.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "network.h"
#include "run.h"
#include "utilities.h"
#include "wrapper.h"

using namespace std;

void BranchNode::step(int& action,
					  bool& is_next,
					  Run* run) {
	bool is_branch;
	if (this->ramp < RAMP_NUM_GEARS) {
		uniform_int_distribution<int> on_distribution(0, RAMP_NUM_GEARS);
		if (this->ramp >= on_distribution(generator)) {
			this->original_network->activate(run->state);
			this->branch_network->activate(run->state);
			if (this->branch_network->output->acti_vals[0] >= this->original_network->output->acti_vals[0]) {
				is_branch = true;
			} else {
				is_branch = false;
			}
		} else {
			is_branch = false;
		}
	} else {
		this->original_network->activate(run->state);
		this->branch_network->activate(run->state);
		if (this->branch_network->output->acti_vals[0] >= this->original_network->output->acti_vals[0]) {
			is_branch = true;
		} else {
			is_branch = false;
		}
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
