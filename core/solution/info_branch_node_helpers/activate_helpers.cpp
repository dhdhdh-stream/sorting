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

	double inner_score;
	this->scope->activate(problem,
						  run_helper,
						  inner_score);

	history->score = inner_score;

	bool is_branch;
	#if defined(MDEBUG) && MDEBUG
	if (run_helper.curr_run_seed%2 == 0) {
		is_branch = true;
	} else {
		is_branch = false;
	}
	run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
	#else
	if (this->is_negate) {
		if (history->score >= 0.0) {
			is_branch = false;
		} else {
			is_branch = true;
		}
	} else {
		if (history->score >= 0.0) {
			is_branch = true;
		} else {
			is_branch = false;
		}
	}
	#endif /* MDEBUG */

	if (is_branch) {
		curr_node = this->branch_next_node;
	} else {
		curr_node = this->original_next_node;
	}

	if (!run_helper.exceeded_limit) {
		for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
			bool is_selected = this->experiments[e_index]->activate(
				this,
				is_branch,
				curr_node,
				problem,
				context,
				run_helper);
			if (is_selected) {
				return;
			}
		}
	}

	if (run_helper.experiments_seen_order.size() == 0) {
		map<Scope*, set<pair<AbstractNode*,bool>>>::iterator scope_it = run_helper.nodes_seen.find(this->parent);
		if (scope_it == run_helper.nodes_seen.end()) {
			scope_it = run_helper.nodes_seen.insert({this->parent, set<pair<AbstractNode*,bool>>()}).first;
		}
		scope_it->second.insert({this, is_branch});
	}
}
