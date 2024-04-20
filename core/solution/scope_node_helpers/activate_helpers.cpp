#include "scope_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "globals.h"
#include "scope.h"

using namespace std;

void ScopeNode::activate(AbstractNode*& curr_node,
						 Problem* problem,
						 vector<ContextLayer>& context,
						 int& exit_depth,
						 AbstractNode*& exit_node,
						 RunHelper& run_helper,
						 map<AbstractNode*, AbstractNodeHistory*>& node_histories) {
	if (num_actions_until_experiment != -1
			&& num_actions_after_experiment_to_skip > 0) {
		num_actions_after_experiment_to_skip--;

		num_actions_until_experiment++;
		/**
		 * - to cancel out later decrement
		 */

		curr_node = this->next_node;
	} else {
		ScopeNodeHistory* history = new ScopeNodeHistory();
		history->index = node_histories.size();
		node_histories[this] = history;

		context.back().node = this;

		context.push_back(ContextLayer());

		context.back().scope = this->scope;
		context.back().node = NULL;

		ScopeHistory* scope_history = new ScopeHistory(this->scope);
		history->scope_history = scope_history;
		context.back().scope_history = scope_history;

		int inner_exit_depth = -1;
		AbstractNode* inner_exit_node = NULL;

		this->scope->activate(problem,
							  context,
							  inner_exit_depth,
							  inner_exit_node,
							  run_helper,
							  scope_history);

		context.pop_back();

		context.back().node = NULL;

		if (run_helper.exceeded_limit) {
			history->normal_exit = false;

			// do nothing
		} else if (inner_exit_depth == -1) {
			history->normal_exit = true;

			curr_node = this->next_node;

			for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
				bool is_selected = this->experiments[e_index]->activate(
					curr_node,
					problem,
					context,
					exit_depth,
					exit_node,
					run_helper);
				if (is_selected) {
					return;
				}
			}
		} else if (inner_exit_depth == 0) {
			history->normal_exit = false;

			curr_node = inner_exit_node;
		} else {
			history->normal_exit = false;

			exit_depth = inner_exit_depth-1;
			exit_node = inner_exit_node;
		}
	}
}
