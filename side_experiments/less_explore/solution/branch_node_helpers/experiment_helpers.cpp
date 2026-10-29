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
	if (this->ramp == 0) {
		wrapper->node_context.back() = this->original_next_node;
		return;
	} else if (this->ramp < MAX_RAMP) {
		map<BranchNode*, bool>::iterator it = wrapper->ramp_tracker[
			this->parent->id].find(this);
		if (it == wrapper->ramp_tracker[this->parent->id].end()) {
			uniform_int_distribution<int> distribution(0, MAX_RAMP);
			bool is_on;
			if (distribution(generator) < this->ramp) {
				is_on = true;
			} else {
				is_on = false;
			}
			it = wrapper->ramp_tracker[this->parent->id].insert({this, is_on}).first;
		}
		if (!it->second) {
			wrapper->node_context.back() = this->original_next_node;
			return;
		}
	}

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

	history->obs = obs;

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
