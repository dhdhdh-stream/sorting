#include "scope_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "constants.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void ScopeNode::result_activate(AbstractNode*& curr_node,
								Problem* problem,
								vector<ContextLayer>& context,
								RunHelper& run_helper) {
	context.back().node_id = this->id;

	this->scope->result_activate(problem,
								 context,
								 run_helper);

	curr_node = this->next_node;

	if (run_helper.experiments_seen_order.size() == 0) {
		if (solution->timestamp >= MAINTAIN_ITERS
				|| (this->parent->id == 0 || this->parent->id > solution->num_existing_scopes)) {
			map<pair<AbstractNode*,bool>, int>::iterator it = run_helper.nodes_seen.find({this, false});
			if (it == run_helper.nodes_seen.end()) {
				run_helper.nodes_seen[{this, false}] = 1;
			} else {
				it->second++;
			}
		}
	}

	for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
		bool is_selected = this->experiments[e_index]->result_activate(
			this,
			false,
			curr_node,
			problem,
			context,
			run_helper);
		if (is_selected) {
			return;
		}
	}
}
