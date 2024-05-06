#if defined(MDEBUG) && MDEBUG

#include "info_branch_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "info_scope.h"
#include "utilities.h"

using namespace std;

void InfoBranchNode::verify_activate(AbstractNode*& curr_node,
									 Problem* problem,
									 vector<ContextLayer>& context,
									 RunHelper& run_helper,
									 map<AbstractNode*, AbstractNodeHistory*>& node_histories) {
	InfoBranchNodeHistory* history = new InfoBranchNodeHistory();
	history->index = node_histories.size();
	node_histories[this] = history;

	bool inner_is_positive;
	this->scope->verify_activate(problem,
								 run_helper,
								 history->scope_history,
								 inner_is_positive);

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
}

#endif /* MDEBUG */