#include "scope_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void ScopeNode::result_activate(AbstractNode*& curr_node,
								Problem* problem,
								vector<ContextLayer>& context,
								RunHelper& run_helper) {
	this->scope->result_activate(problem,
								 context,
								 run_helper);

	curr_node = this->next_node;

	run_helper.num_actions++;
	if (run_helper.num_actions > solution->num_actions_limit) {
		run_helper.exceeded_limit = true;
		return;
	}
	if (run_helper.experiments_seen_order.size() == 0) {
		if (solution->subproblem_id == -1
				|| this->parent->id >= solution->subproblem_id) {
			map<pair<AbstractNode*,bool>, int>::iterator it = run_helper.nodes_seen.find({this, false});
			if (it == run_helper.nodes_seen.end()) {
				run_helper.nodes_seen[{this, false}] = 1;
			} else {
				it->second++;
			}
		}
	}
	context.back().location_history[this] = problem->get_location();

	if (!run_helper.exceeded_limit) {
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
}
