#include "action_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

void ActionNode::activate(AbstractNode*& curr_node,
						  Problem* problem,
						  vector<ContextLayer>& context,
						  RunHelper& run_helper,
						  map<AbstractNode*, AbstractNodeHistory*>& node_histories) {
	if (solution->num_actions_until_experiment != -1
			&& run_helper.num_actions_after_experiment_to_skip > 0) {
		curr_node = this->next_node;
	} else {
		ActionNodeHistory* history = new ActionNodeHistory();
		history->index = (int)node_histories.size();
		node_histories[this] = history;

		problem->perform_action(this->action);
		history->obs_snapshot = problem->get_observations();

		curr_node = this->next_node;

		for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
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
