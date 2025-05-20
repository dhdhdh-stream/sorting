#include "branch_node.h"

#include "abstract_experiment.h"
#include "scope.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

void BranchNode::result_activate(AbstractNode*& curr_node,
								 Problem* problem,
								 RunHelper& run_helper,
								 ScopeHistory* scope_history) {
	BranchNodeHistory* history = new BranchNodeHistory(this);
	history->index = (int)scope_history->node_histories.size();
	scope_history->node_histories[this->id] = history;

	double sum_vals = this->average_val;
	for (int f_index = 0; f_index < (int)this->factor_ids.size(); f_index++) {
		double val;
		fetch_factor_helper(scope_history,
							this->factor_ids[f_index],
							val);
		sum_vals += this->factor_weights[f_index] * val;
	}

	bool is_branch;
	#if defined(MDEBUG) && MDEBUG
	if (run_helper.curr_run_seed%2 == 0) {
		is_branch = true;
	} else {
		is_branch = false;
	}
	run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
	#else
	if (sum_vals >= 0.0) {
		is_branch = true;
	} else {
		is_branch = false;
	}
	#endif /* MDEBUG */

	history->is_branch = is_branch;

	if (is_branch) {
		curr_node = this->branch_next_node;
	} else {
		curr_node = this->original_next_node;
	}

	if (this->experiment != NULL) {
		this->experiment->result_activate(
			this,
			is_branch,
			curr_node,
			problem,
			run_helper,
			scope_history);
	}
}
