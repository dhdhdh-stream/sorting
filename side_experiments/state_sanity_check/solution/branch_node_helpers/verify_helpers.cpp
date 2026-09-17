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

const double MAX_DIFF = 0.0000001;

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
			bool mismatch = false;
			vector<double> original_states;
			for (int s_index = 0; s_index < (int)this->original_network->init_states.size(); s_index++) {
				original_states.push_back(wrapper->states.back()(this->original_network->init_states[s_index]));

				if (abs(original_states[s_index] - this->verify_original_state_vals[0][s_index]) > MAX_DIFF) {
					mismatch = true;
				}
			}
			if (mismatch) {
				cout << "this->parent->id: " << this->parent->id << endl;
				cout << "this->id: " << this->id << endl;
				cout << "original_states:";
				for (int s_index = 0; s_index < (int)original_states.size(); s_index++) {
					cout << " " << original_states[s_index];
				}
				cout << endl;
				cout << "this->verify_original_state_vals[0]:";
				for (int s_index = 0; s_index < (int)this->verify_original_state_vals[0].size(); s_index++) {
					cout << " " << this->verify_original_state_vals[0][s_index];
				}
				cout << endl;
				cout << "wrapper->starting_run_seed: " << wrapper->starting_run_seed << endl;
				throw invalid_argument("original_states != this->verify_original_state_vals[0]");
			}

			this->verify_original_state_vals.erase(this->verify_original_state_vals.begin());
		}

		if (this->verify_branch_state_vals.size() > 0) {
			bool mismatch = false;
			vector<double> branch_states;
			for (int s_index = 0; s_index < (int)this->branch_network->init_states.size(); s_index++) {
				branch_states.push_back(wrapper->states.back()(this->branch_network->init_states[s_index]));

				if (abs(branch_states[s_index] - this->verify_branch_state_vals[0][s_index]) > MAX_DIFF) {
					mismatch = true;
				}
			}
			if (mismatch) {
				cout << "this->parent->id: " << this->parent->id << endl;
				cout << "this->id: " << this->id << endl;
				cout << "branch_states:";
				for (int s_index = 0; s_index < (int)branch_states.size(); s_index++) {
					cout << " " << branch_states[s_index];
				}
				cout << endl;
				cout << "this->verify_branch_state_vals[0]:";
				for (int s_index = 0; s_index < (int)this->verify_branch_state_vals[0].size(); s_index++) {
					cout << " " << this->verify_branch_state_vals[0][s_index];
				}
				cout << endl;
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