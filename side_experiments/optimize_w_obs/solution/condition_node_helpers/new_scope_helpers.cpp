#include "condition_node.h"

#include "abstract_experiment.h"
#include "globals.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

void ConditionNode::new_scope_activate(AbstractNode*& curr_node,
									   Problem* problem,
									   vector<ContextLayer>& context,
									   RunHelper& run_helper,
									   ScopeHistory* scope_history) {
	ConditionNodeHistory* history = new ConditionNodeHistory(this);
	history->index = (int)scope_history->node_histories.size();
	scope_history->node_histories[this->id] = history;

	bool is_match = true;
	for (int c_index = 0; c_index < (int)this->conditions.size(); c_index++) {
		pair<pair<vector<int>,vector<int>>,int> input = {this->conditions[c_index].first, -1};
		double obs = 0.0;
		fetch_input_helper(scope_history,
						   input,
						   0,
						   obs);
		if (this->conditions[c_index].second) {
			if (obs != 1.0) {
				is_match = false;
				break;
			}
		} else {
			if (obs != -1.0) {
				is_match = false;
				break;
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
}
