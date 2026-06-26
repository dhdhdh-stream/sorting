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
	if (this->consec_original >= CONSEC_DEPRECATE_LIMIT) {
		wrapper->node_context.back() = this->original_next_node;
	} else if (this->consec_branch >= CONSEC_DEPRECATE_LIMIT) {
		wrapper->node_context.back() = this->branch_next_node;
	} else {
		ScopeHistory* scope_history = wrapper->scope_histories.back();

		BranchNodeHistory* history = new BranchNodeHistory(this);
		history->index = (int)scope_history->node_histories.size();
		scope_history->node_histories[this->id] = history;

		bool is_branch;
		if (this->ramp < RAMP_NUM_GEARS) {
			uniform_int_distribution<int> on_distribution(0, RAMP_NUM_GEARS);
			if (this->ramp >= on_distribution(generator)) {
				this->original_network->activate(obs);
				this->branch_network->activate(obs);
				if (this->branch_network->output->acti_vals[0] >= this->original_network->output->acti_vals[0]) {
					is_branch = true;
				} else {
					is_branch = false;
				}
			} else {
				is_branch = false;
			}
		} else {
			this->original_network->activate(obs);
			this->branch_network->activate(obs);
			if (this->branch_network->output->acti_vals[0] >= this->original_network->output->acti_vals[0]) {
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
			map<BranchNode*, pair<int,pair<bool,vector<double>>>>::iterator it = wrapper->branch_node_samples.find(this);
			if (it == wrapper->branch_node_samples.end()) {
				it = wrapper->branch_node_samples.insert({this, {0, {false,vector<double>()}}}).first;
			}
			uniform_int_distribution<int> select_distribution(0, it->second.first);
			if (select_distribution(generator) == 0) {
				it->second.second.first = is_branch;
				it->second.second.second = obs;
			}
			it->second.first++;
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
}
