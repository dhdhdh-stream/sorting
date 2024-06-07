#include "scope_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void ScopeNode::activate(AbstractNode*& curr_node,
						 Problem* problem,
						 vector<ContextLayer>& context,
						 RunHelper& run_helper,
						 map<AbstractNode*, AbstractNodeHistory*>& node_histories) {
	context.back().node = this;

	this->scope->activate(problem,
						  context,
						  run_helper);

	context.back().node = NULL;

	ScopeNodeHistory* history = new ScopeNodeHistory();
	history->index = node_histories.size();
	node_histories[this] = history;
	history->obs_snapshot = problem->get_observations();

	curr_node = this->next_node;

	if (!run_helper.exceeded_limit) {
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
