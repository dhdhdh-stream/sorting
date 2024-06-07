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
						  history->is_positive);

	if (!run_helper.exceeded_limit) {
		curr_node = this->next_node;

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

		uniform_int_distribution<int> swap_distribution(0, run_helper.num_actions-1);
		if (swap_distribution(generator) == 0) {
			run_helper.explore_node = this;
			run_helper.explore_is_branch = false;
		}
	}
}
