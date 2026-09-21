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

	history->obs = obs;

	if (is_branch) {
		wrapper->node_context.back() = this->branch_next_node;

		for (int e_index = 0; e_index < (int)this->branch_experiments.size(); e_index++) {
			this->branch_experiments[e_index]->experiment_check_activate(
				obs,
				wrapper);
		}
	} else {
		wrapper->node_context.back() = this->original_next_node;

		for (int e_index = 0; e_index < (int)this->original_experiments.size(); e_index++) {
			this->original_experiments[e_index]->experiment_check_activate(
				obs,
				wrapper);
		}
	}
}
