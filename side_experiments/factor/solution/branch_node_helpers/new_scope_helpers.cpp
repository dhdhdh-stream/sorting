#if defined(MDEBUG) && MDEBUG

#include "branch_node.h"

#include <iostream>

#include "problem.h"
#include "scope.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

void BranchNode::new_scope_capture_verify_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		RunHelper& run_helper,
		ScopeHistory* scope_history) {
	BranchNodeHistory* history = new BranchNodeHistory(this);
	scope_history->node_histories[this->id] = history;

	double sum_vals = this->average_val;
	for (int f_index = 0; f_index < (int)this->factor_ids.size(); f_index++) {
		double val;
		fetch_factor_helper(run_helper,
							scope_history,
							this->factor_ids[f_index],
							val);
		sum_vals += this->factor_weights[f_index] * val;
	}

	this->verify_scores.push_back(sum_vals);

	cout << "run_helper.starting_run_seed: " << run_helper.starting_run_seed << endl;
	cout << "run_helper.curr_run_seed: " << run_helper.curr_run_seed << endl;
	problem->print();

	bool is_branch;
	if (run_helper.curr_run_seed%2 == 0) {
		is_branch = true;
	} else {
		is_branch = false;
	}
	run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);

	history->is_branch = is_branch;

	if (is_branch) {
		curr_node = this->branch_next_node;
	} else {
		curr_node = this->original_next_node;
	}
}

#endif /* MDEBUG */