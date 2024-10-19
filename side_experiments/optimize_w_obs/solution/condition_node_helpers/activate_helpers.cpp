#include "condition_node.h"

#include "globals.h"
#include "solution.h"

using namespace std;

void ConditionNode::activate(AbstractNode*& curr_node,
							 Problem* problem,
							 vector<ContextLayer>& context,
							 RunHelper& run_helper) {
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
