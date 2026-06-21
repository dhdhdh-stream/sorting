#include "branch_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "constants.h"
#include "experiment_run.h"
#include "globals.h"
#include "network.h"
#include "utilities.h"
#include "wrapper.h"

using namespace std;

void BranchNode::experiment_step(int& action,
								 bool& is_next,
								 ExperimentRun* run) {
	BranchNodeHistory* history = new BranchNodeHistory(this);
	history->index = (int)run->node_histories.size();
	run->node_histories[this->id] = history;

	history->state = run->state;
	history->obs_history_index = run->obs_histories.size();

	this->original_network->activate(run->state);
	this->branch_network->activate(run->state);

	bool is_branch;
	if (this->ramp < RAMP_NUM_GEARS) {
		uniform_int_distribution<int> on_distribution(0, RAMP_NUM_GEARS);
		if (this->ramp >= on_distribution(generator)) {
			if (this->branch_network->output->acti_vals[0] >= this->original_network->output->acti_vals[0]) {
				is_branch = true;
			} else {
				is_branch = false;
			}
		} else {
			is_branch = false;
		}
	} else {
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
		history->is_branch = true;

		if (!run->should_force) {
			run->taken_branch_node_networks.back().push_back(this->branch_network);
		}

		run->node_context = this->branch_next_node;

		if (this->branch_experiment != NULL
				&& run->should_force) {
			this->branch_experiment->experiment_activate(run);
		}
	} else {
		history->is_branch = false;

		if (!run->should_force) {
			run->taken_branch_node_networks.back().push_back(this->original_network);
		}

		run->node_context = this->original_next_node;

		if (this->original_experiment != NULL
				&& run->should_force) {
			this->original_experiment->experiment_activate(run);
		}
	}
}

void BranchNode::experiment_step_start(ExperimentRun* run) {
	// unreachable
}
