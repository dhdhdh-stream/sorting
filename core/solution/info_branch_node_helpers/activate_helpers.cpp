#include "info_branch_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "globals.h"
#include "info_scope.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

void InfoBranchNode::activate(AbstractNode*& curr_node,
							  Problem* problem,
							  vector<ContextLayer>& context,
							  RunHelper& run_helper,
							  map<AbstractNode*, AbstractNodeHistory*>& node_histories) {
	InfoBranchNodeHistory* history = new InfoBranchNodeHistory();
	history->index = node_histories.size();
	node_histories[this] = history;

	bool inner_is_positive;
	this->scope->activate(problem,
						  run_helper,
						  inner_is_positive);

	if (!run_helper.exceeded_limit) {
		if (this->is_negate) {
			if (inner_is_positive) {
				history->is_branch = false;
			} else {
				history->is_branch = true;
			}
		} else {
			if (inner_is_positive) {
				history->is_branch = true;
			} else {
				history->is_branch = false;
			}
		}

		if (history->is_branch) {
			curr_node = this->branch_next_node;
		} else {
			curr_node = this->original_next_node;
		}

		for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
			bool is_selected = this->experiments[e_index]->activate(
				this,
				history->is_branch,
				curr_node,
				problem,
				context,
				run_helper);
			if (is_selected) {
				return;
			}
		}

		uniform_int_distribution<int> swap_distribution(0, run_helper.num_actions-1);
		if (swap_distribution(generator) == 0) {
			run_helper.explore_node = this;
			run_helper.explore_is_branch = history->is_branch;
		}
	}
}
