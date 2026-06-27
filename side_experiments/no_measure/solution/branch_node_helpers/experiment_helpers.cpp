#include "branch_node.h"

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
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	BranchNodeHistory* history = new BranchNodeHistory(this);
	history->index = (int)scope_history->node_histories.size();
	scope_history->node_histories[this->id] = history;

	bool is_branch;
	this->original_network->activate(obs);
	this->branch_network->activate(obs);
	uniform_int_distribution<int> insanity_distribution(0, 99);
	if (this->branch_network->output->acti_vals[0] >= this->original_network->output->acti_vals[0]) {
		if (insanity_distribution(generator) == 0) {
			is_branch = false;
		} else {
			is_branch = true;
		}
	} else {
		if (insanity_distribution(generator) == 0) {
			is_branch = true;
		} else {
			is_branch = false;
		}
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
		wrapper->node_context.back() = this->branch_next_node;
	} else {
		wrapper->node_context.back() = this->original_next_node;
	}

	if (this->experiment != NULL) {
		this->experiment->experiment_check_activate(
			this,
			is_branch,
			wrapper);
	}
}
