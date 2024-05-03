#include "info_branch_node.h"

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
						  history->scope_history,
						  inner_is_positive);

	#if defined(MDEBUG) && MDEBUG
	if (run_helper.curr_run_seed%2 == 0) {
		history->is_branch = true;
	} else {
		history->is_branch = false;
	}
	run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
	#else
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
	#endif /* MDEBUG */

	if (history->is_branch) {
		curr_node = this->branch_next_node;
	} else {
		curr_node = this->original_next_node;
	}

	for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
		if (this->experiments[e_index]->is_branch == history->is_branch) {
			bool is_selected = this->experiments[e_index]->activate(
				curr_node,
				problem,
				context,
				run_helper);
			if (is_selected) {
				return;
			}
		}
	}
}
