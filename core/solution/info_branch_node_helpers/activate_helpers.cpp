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
	if (run_helper.branch_node_ancestors.find(this) != run_helper.branch_node_ancestors.end()) {
		curr_node = this->original_next_node;
	} else {
		run_helper.branch_node_ancestors.insert(this);

		InfoBranchNodeHistory* history = new InfoBranchNodeHistory();
		history->index = node_histories.size();
		node_histories[this] = history;

		bool is_positive;
		this->scope->activate(problem,
							  context,
							  run_helper,
							  is_positive);

		if (this->is_negate) {
			if (is_positive) {
				history->is_branch = false;
			} else {
				history->is_branch = true;
			}
		} else {
			if (is_positive) {
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

		if (!run_helper.exceeded_limit) {
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
		}

		if (run_helper.experiments_seen_order.size() == 0) {
			map<pair<AbstractNode*,bool>, int>::iterator it = run_helper.scope_nodes_seen.find({this, history->is_branch});
			if (it == run_helper.scope_nodes_seen.end()) {
				run_helper.scope_nodes_seen[{this, history->is_branch}] = 1;
			} else {
				it->second++;
			}
		}
	}
}
