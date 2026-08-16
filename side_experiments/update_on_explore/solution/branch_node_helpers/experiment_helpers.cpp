#include "branch_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "scope.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "utilities.h"

using namespace std;

void BranchNode::experiment_step(vector<double>& obs,
								 int& action,
								 bool& is_next,
								 SolutionWrapper* wrapper) {
	if (this->consec_original >= CONSEC_DEPRECATE_LIMIT) {
		wrapper->node_context.back() = this->original_next_node;
		return;
	}
	if (this->consec_branch >= CONSEC_DEPRECATE_LIMIT) {
		wrapper->node_context.back() = this->branch_next_node;
		return;
	}

	ScopeHistory* scope_history = wrapper->scope_histories.back();

	BranchNodeHistory* history = new BranchNodeHistory(this);
	history->index = (int)scope_history->node_histories.size();
	scope_history->node_histories[this->id] = history;

	bool is_branch;
	this->original_network->activate(obs);
	double original_predicted = this->original_network->output->acti_vals[0];
	this->branch_network->activate(obs);
	double branch_predicted = this->branch_network->output->acti_vals[0];
	if (branch_predicted >= original_predicted) {
		is_branch = true;
	} else {
		is_branch = false;
	}

	#if defined(MDEBUG) && MDEBUG
	if (wrapper->curr_run_seed%2 == 0) {
		is_branch = true;
	} else {
		is_branch = false;
	}
	wrapper->curr_run_seed = xorshift(wrapper->curr_run_seed);
	#endif /* MDEBUG */

	history->is_branch = is_branch;

	if (!wrapper->should_explore) {
		history->obs = obs;
	}

	if (is_branch) {
		if (!wrapper->should_explore) {
			this->consec_original = 0;
			this->consec_branch++;
		}

		wrapper->node_context.back() = this->branch_next_node;

		if (this->branch_experiment != NULL
				&& this->branch_experiment->diversity_index == wrapper->diversity_index) {
			this->branch_experiment->experiment_check_activate(
				obs,
				wrapper);
		}
	} else {
		if (!wrapper->should_explore) {
			this->consec_original++;
			this->consec_branch = 0;
		}

		wrapper->node_context.back() = this->original_next_node;

		if (this->original_experiment != NULL
				&& this->original_experiment->diversity_index == wrapper->diversity_index) {
			this->original_experiment->experiment_check_activate(
				obs,
				wrapper);
		}
	}
}
