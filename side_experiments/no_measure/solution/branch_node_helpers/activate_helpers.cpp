#include "branch_node.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "utilities.h"

using namespace std;

void BranchNode::step(vector<double>& obs,
					  int& action,
					  bool& is_next,
					  SolutionWrapper* wrapper) {
	uniform_int_distribution<int> on_distribution(0, this->ramp_num_gears);
	if (this->consec_original >= CONSEC_DEPRECATE_LIMIT) {
		wrapper->node_context.back() = this->original_next_node;
	} else if (this->consec_branch >= CONSEC_DEPRECATE_LIMIT) {
		wrapper->node_context.back() = this->branch_next_node;
	} else if (this->ramp < this->ramp_num_gears
			&& this->ramp < on_distribution(generator)) {
		wrapper->node_context.back() = this->original_next_node;
	} else {
		ScopeHistory* scope_history = wrapper->scope_histories.back();

		BranchNodeHistory* history = new BranchNodeHistory(this);
		history->index = (int)scope_history->node_histories.size();
		scope_history->node_histories[this->id] = history;

		bool is_branch;
		for (int l_index = (int)this->original_networks.size()-1; l_index >= 0; l_index--) {
			if (l_index == 0) {
				this->original_networks[l_index]->activate(obs);
				this->branch_networks[l_index]->activate(obs);
				if (this->branch_networks[l_index]->output->acti_vals[0] >= this->original_networks[l_index]->output->acti_vals[0]) {
					is_branch = true;
				} else {
					is_branch = false;
				}
			} else {
				uniform_int_distribution<int> distribution(0, MAINTAIN_NUM_ITERS-1);
				if (distribution(generator) <= this->maintain_iters[l_index]) {
					this->original_networks[l_index]->activate(obs);
					this->branch_networks[l_index]->activate(obs);
					if (this->branch_networks[l_index]->output->acti_vals[0] >= this->original_networks[l_index]->output->acti_vals[0]) {
						is_branch = true;
					} else {
						is_branch = false;
					}
					break;
				}
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

		if (is_branch) {
			wrapper->node_context.back() = this->branch_next_node;
		} else {
			wrapper->node_context.back() = this->original_next_node;
		}
	}
}
