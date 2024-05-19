#include "info_scope_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "globals.h"
#include "info_scope.h"
#include "solution.h"

using namespace std;

void InfoScopeNode::activate(AbstractNode*& curr_node,
							 Problem* problem,
							 vector<ContextLayer>& context,
							 RunHelper& run_helper,
							 map<AbstractNode*, AbstractNodeHistory*>& node_histories) {
	InfoScopeNodeHistory* history = new InfoScopeNodeHistory();
	history->index = node_histories.size();
	node_histories[this] = history;

	this->scope->activate(problem,
						  run_helper,
						  history->scope_history,
						  history->is_positive);

	curr_node = this->next_node;

	if (run_helper.num_actions < solution->num_actions_limit) {
		for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
			bool is_selected = this->experiments[e_index]->activate(
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
