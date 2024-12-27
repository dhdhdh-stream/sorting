#include "action_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "constants.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void ActionNode::result_activate(AbstractNode*& curr_node,
								 Problem* problem,
								 vector<ContextLayer>& context,
								 RunHelper& run_helper) {
	problem->perform_action(this->action);

	if (this->input_scope_contexts.size() > 0) {
		vector<double> obs = problem->get_observations();

		for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
			bool match_context = false;
			if (context.size() >= this->input_scope_contexts[i_index].size()) {
				match_context = true;
				for (int l_index = 0; l_index < (int)this->input_scope_contexts[i_index].size()-1; l_index++) {
					int context_index = context.size() - this->input_scope_contexts[i_index].size() + l_index;
					if (context[context_index].scope != this->input_scope_contexts[i_index][l_index]
							|| context[context_index].node_id != this->input_node_context_ids[i_index][l_index]) {
						match_context = false;
						break;
					}
				}
			}

			if (match_context) {
				if (this->input_obs_indexes[i_index] == -1) {
					context[context.size() - this->input_scope_contexts[i_index].size()]
						.obs_history[{{this->input_scope_contexts[i_index],
							this->input_node_context_ids[i_index]}, -1}] = 1.0;
				} else {
					context[context.size() - this->input_scope_contexts[i_index].size()]
						.obs_history[{{this->input_scope_contexts[i_index],
							this->input_node_context_ids[i_index]}, this->input_obs_indexes[i_index]}] = obs[this->input_obs_indexes[i_index]];
				}
			}
		}
	}

	curr_node = this->next_node;

	if (run_helper.experiments_seen_order.size() == 0) {
		if (!solution->was_commit || this->was_commit) {
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
