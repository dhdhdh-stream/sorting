#if defined(MDEBUG) && MDEBUG

#include "branch_node.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "init_network.h"
#include "problem.h"
#include "scope.h"
#include "score_network.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "utilities.h"

using namespace std;

void BranchNode::verify_step(SolutionWrapper* wrapper) {
	if (this->consec_original >= CONSEC_DEPRECATE_LIMIT) {
		wrapper->node_context.back() = this->original_next_node;
	} else if (this->consec_branch >= CONSEC_DEPRECATE_LIMIT) {
		wrapper->node_context.back() = this->branch_next_node;
	} else {
		ScopeHistory* scope_history = wrapper->scope_histories.back();

		BranchNodeHistory* history = new BranchNodeHistory(this);
		scope_history->node_histories.push_back(history);

		if (this->verify_original_state_vals.size() > 0) {
			vector<double> original_states;
			for (int s_index = 0; s_index < (int)this->original_network->init_states.size(); s_index++) {
				original_states.push_back(wrapper->states.back()(this->original_network->init_states[s_index]));
			}
			if (original_states != this->verify_original_state_vals[0]) {
				cout << "wrapper->starting_run_seed: " << wrapper->starting_run_seed << endl;
				throw invalid_argument("original_states != this->verify_original_state_vals[0]");
			}

			this->verify_original_state_vals.erase(this->verify_original_state_vals.begin());
		}

		if (this->verify_branch_state_vals.size() > 0) {
			vector<double> branch_states;
			for (int s_index = 0; s_index < (int)this->branch_network->init_states.size(); s_index++) {
				branch_states.push_back(wrapper->states.back()(this->branch_network->init_states[s_index]));
			}
			if (branch_states != this->verify_branch_state_vals[0]) {
				cout << "wrapper->starting_run_seed: " << wrapper->starting_run_seed << endl;
				throw invalid_argument("branch_states != this->verify_branch_state_vals[0]");
			}

			this->verify_branch_state_vals.erase(this->verify_branch_state_vals.begin());
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
}

#endif /* MDEBUG */