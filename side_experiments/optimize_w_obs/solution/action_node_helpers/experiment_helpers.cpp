#include "action_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void ActionNode::experiment_activate(AbstractNode*& curr_node,
									 Problem* problem,
									 vector<ContextLayer>& context,
									 RunHelper& run_helper,
									 ScopeHistory* scope_history) {
	ActionNodeHistory* history = new ActionNodeHistory(this);
	history->index = (int)scope_history->node_histories.size();
	scope_history->node_histories[this->id] = history;

	problem->perform_action(this->action);

	vector<double> obs = problem->get_observations();
	history->obs_history = obs;

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
				context[context.size() - this->input_scope_context_ids[i_index].size()]
					.obs_history[{{this->input_scope_context_ids[i_index],
						this->input_node_context_ids[i_index]}, -1}] = 1.0;
			} else {
				context[context.size() - this->input_scope_context_ids[i_index].size()]
					.obs_history[{{this->input_scope_context_ids[i_index],
						this->input_node_context_ids[i_index]}, this->input_obs_indexes[i_index]}] = obs[this->input_obs_indexes[i_index]];
			}
		}
	}

	curr_node = this->next_node;

	run_helper.num_actions++;
	if (run_helper.num_actions > solution->num_actions_limit) {
		run_helper.exceeded_limit = true;
		return;
	}
	if (run_helper.experiments_seen_order.size() == 0) {
		map<pair<AbstractNode*,bool>, int>::iterator it = run_helper.nodes_seen.find({this, false});
		if (it == run_helper.nodes_seen.end()) {
			run_helper.nodes_seen[{this, false}] = 1;
		} else {
			it->second++;
		}
	}

	for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
		bool is_selected = this->experiments[e_index]->activate(
			this,
			false,
			curr_node,
			problem,
			context,
			run_helper,
			scope_history);
		if (is_selected) {
			return;
		}
	}
}
