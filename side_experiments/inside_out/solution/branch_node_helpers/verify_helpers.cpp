#if defined(MDEBUG) && MDEBUG

#include "branch_node.h"

#include <iostream>

#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "utilities.h"

using namespace std;

void BranchNode::verify_step(vector<double>& obs,
							 int& action,
							 bool& is_next,
							 SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

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

	if (this->verify_key != NULL) {
		cout << "this->id: " << this->id << endl;

		cout << "wrapper->starting_run_seed: " << wrapper->starting_run_seed << endl;
		cout << "wrapper->curr_run_seed: " << wrapper->curr_run_seed << endl;
		wrapper->problem->print();

		if (this->verify_scores[0] != sum_vals) {
			cout << "this->verify_scores[0]: " << this->verify_scores[0] << endl;
			cout << "sum_vals: " << sum_vals << endl;

			cout << "seed: " << seed << endl;

			throw invalid_argument("branch node verify fail");
		}

		this->verify_scores.erase(this->verify_scores.begin());
	}

	bool is_branch;
	if (wrapper->curr_run_seed%2 == 0) {
		is_branch = true;
	} else {
		is_branch = false;
	}
	wrapper->curr_run_seed = xorshift(wrapper->curr_run_seed);

	history->is_branch = is_branch;

	if (is_branch) {
		wrapper->node_context.back() = this->branch_next_node;
	} else {
		wrapper->node_context.back() = this->original_next_node;
	}
}

#endif /* MDEBUG */