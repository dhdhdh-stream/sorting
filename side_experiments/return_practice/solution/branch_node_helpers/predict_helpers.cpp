#include "branch_node.h"

#include <iostream>

#include "constants.h"
#include "experiment.h"
#include "network.h"
#include "predict_run.h"
#include "utilities.h"
#include "wrapper.h"

using namespace std;

void BranchNode::predict_step(PredictRun* run) {
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

		if (this->branch_experiment != NULL) {
			this->branch_experiment->predict_activate(run);
		}
	} else {
		run->node_context = this->original_next_node;

		if (this->original_experiment != NULL) {
			this->original_experiment->predict_activate(run);
		}
	}
}
