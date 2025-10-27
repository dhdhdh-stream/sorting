#if defined(MDEBUG) && MDEBUG

#include "branch_node.h"

#include <iostream>

#include "globals.h"
#include "network.h"
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
	scope_history->node_histories[this->id] = history;

	this->network->activate(obs);

	if (this->verify_key != NULL) {
		cout << "this->id: " << this->id << endl;

		cout << "wrapper->starting_run_seed: " << wrapper->starting_run_seed << endl;
		cout << "wrapper->curr_run_seed: " << wrapper->curr_run_seed << endl;
		wrapper->problem->print();

		if (this->verify_scores[0] != this->network->output->acti_vals[0]) {
			cout << "this->verify_scores[0]: " << this->verify_scores[0] << endl;
			cout << "this->network->output->acti_vals[0]: " << this->network->output->acti_vals[0] << endl;

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