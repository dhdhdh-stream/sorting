#include "branch_node.h"

#include <iostream>

#include "constants.h"
#include "experiment.h"
#include "network.h"
#include "utilities.h"
#include "wrapper.h"

using namespace std;

void BranchNode::experiment_step(vector<double>& obs,
								 int& action,
								 bool& is_next,
								 ExperimentRun* run) {
	BranchNodeHistory* history = new BranchNodeHistory(this);
	run->node_histories[this->id] = history;

	history->obs = obs;

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
		if (this->branch_state_history.size() < STATE_HISTORY_NUM_SAVE) {
			this->branch_state_history.push_back(run->state);
		} else {
			this->branch_state_history[this->branch_state_history_index] = run->state;
		}
		this->branch_state_history_index++;
		if (this->branch_state_history_index >= STATE_HISTORY_NUM_SAVE) {
			this->branch_state_history_index = 0;
		}

		history->is_branch = true;

		run->node_context = this->branch_next_node;

		if (this->branch_experiment != NULL) {
			this->branch_experiment->experiment_activate(run);
		}
	} else {
		if (this->original_state_history.size() < STATE_HISTORY_NUM_SAVE) {
			this->original_state_history.push_back(run->state);
		} else {
			this->original_state_history[this->original_state_history_index] = run->state;
		}
		this->original_state_history_index++;
		if (this->original_state_history_index >= STATE_HISTORY_NUM_SAVE) {
			this->original_state_history_index = 0;
		}

		history->is_branch = false;

		run->node_context = this->original_next_node;

		if (this->original_experiment != NULL) {
			this->original_experiment->experiment_activate(run);
		}
	}
}
