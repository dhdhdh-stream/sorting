#include "condition_node.h"

#include "abstract_experiment.h"
#include "globals.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void ConditionNode::experiment_activate(AbstractNode*& curr_node,
										Problem* problem,
										vector<ContextLayer>& context,
										RunHelper& run_helper,
										ScopeHistory* scope_history) {
	ConditionNodeHistory* history = new ConditionNodeHistory(this);
	history->index = (int)scope_history->node_histories.size();
	scope_history->node_histories[this->id] = history;

	bool is_match = true;
	for (int c_index = 0; c_index < (int)this->conditions.size(); c_index++) {
		map<pair<pair<vector<int>,vector<int>>,int>, double>::iterator it =
			context.back().obs_history.find({this->conditions[c_index].first, -1});
		if (it == context.back().obs_history.end()) {
			is_match = false;
			break;
		} else {
			if (this->conditions[c_index].second) {
				if (it->second != 1.0) {
					is_match = false;
					break;
				}
			} else {
				if (it->second != -1.0) {
					is_match = false;
					break;
				}
			}
		}
	}

	history->is_branch = is_match;

	if (is_match) {
		curr_node = this->branch_next_node;
	} else {
		curr_node = this->original_next_node;
	}

	run_helper.num_actions++;
	if (run_helper.num_actions > solution->num_actions_limit) {
		run_helper.exceeded_limit = true;
		return;
	}

	for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
		bool is_selected = this->experiments[e_index]->activate(
			this,
			is_match,
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
