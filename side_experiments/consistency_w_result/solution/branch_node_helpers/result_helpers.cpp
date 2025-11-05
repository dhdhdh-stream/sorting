#include "branch_node.h"

#include "abstract_experiment.h"
#include "constants.h"
#include "network.h"
#include "scope.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "utilities.h"

using namespace std;

void BranchNode::result_step(vector<double>& obs,
								 int& action,
								 bool& is_next,
								 SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	BranchNodeHistory* history = new BranchNodeHistory(this);
	scope_history->node_histories[this->id] = history;

	if (this->consistency_network != NULL) {
		this->consistency_network->activate(obs);
	}

	this->val_network->activate(obs);

	bool is_branch;
	#if defined(MDEBUG) && MDEBUG
	if (wrapper->curr_run_seed%2 == 0) {
		is_branch = true;
	} else {
		is_branch = false;
	}
	wrapper->curr_run_seed = xorshift(wrapper->curr_run_seed);
	#else
	if ((this->consistency_network == NULL || this->consistency_network->output->acti_vals[0] >= CONSISTENCY_MIN_WEIGHT)
			&& this->val_network->output->acti_vals[0] >= 0.0) {
		is_branch = true;
	} else {
		is_branch = false;
	}
	#endif /* MDEBUG */

	history->is_branch = is_branch;

	if (is_branch) {
		wrapper->node_context.back() = this->branch_next_node;
	} else {
		wrapper->node_context.back() = this->original_next_node;
	}

	if (this->experiment != NULL) {
		this->experiment->result_check_activate(
			this,
			is_branch,
			wrapper);
	}
}
