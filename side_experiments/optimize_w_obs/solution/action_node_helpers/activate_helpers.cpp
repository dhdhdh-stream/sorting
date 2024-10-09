#include "action_node.h"

#include <iostream>

#include "globals.h"
#include "problem.h"
#include "solution.h"
#include "world_model.h"

using namespace std;

void ActionNode::activate(AbstractNode*& curr_node,
						  Problem* problem,
						  vector<ContextLayer>& context,
						  RunHelper& run_helper) {
	problem->perform_action(this->action);

	if (this->input_scope_context_ids.size() > 0) {
		vector<double> obs = problem->get_observations();

		for (int i_index = 0; i_index < (int)this->input_scope_context_ids.size(); i_index++) {
			bool match_context = false;
			if (context.size() >= this->input_scope_context_ids[i_index].size()) {
				match_context = true;
				for (int l_index = 0; l_index < (int)this->input_scope_context_ids[i_index].size()-1; l_index++) {
					int context_index = context.size() - this->input_scope_context_ids[i_index].size() + l_index;
					if (context[context_index].scope_id != this->input_scope_context_ids[i_index][l_index]
							|| context[context_index].node_id != this->input_node_context_ids[i_index][l_index]) {
						match_context = false;
						break;
					}
				}
			}

			if (match_context) {
				if (this->input_obs_indexes[i_index] == -1) {
					context[context.size() - this->input_scope_context_ids.size()]
						.obs_history[{{this->input_scope_context_ids[i_index],
							this->input_node_context_ids[i_index]}, -1}] = 1.0;
				} else {
					context[context.size() - this->input_scope_context_ids.size()]
						.obs_history[{{this->input_scope_context_ids[i_index],
							this->input_node_context_ids[i_index]}, -1}] = obs[this->input_obs_indexes[i_index]];
				}
			}
		}
	}

	curr_node = this->next_node;

	run_helper.num_actions++;
	if (run_helper.num_actions > solution->num_actions_limit) {
		run_helper.exceeded_limit = true;
		return;
	}
}
