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

	double inner_score;
	this->scope->verify_activate(problem,
								 run_helper,
								 inner_score);

	history->score = inner_score;

	bool is_branch;
	if (run_helper.curr_run_seed%2 == 0) {
		is_branch = true;
	} else {
		is_branch = false;
	}
	run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);

	if (is_branch) {
		curr_node = this->branch_next_node;
	} else {
		curr_node = this->original_next_node;
	}
}

#endif /* MDEBUG */