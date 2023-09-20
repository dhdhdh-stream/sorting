#include "scope.h"

using namespace std;

void Scope::activate(vector<int>& starting_node_ids,
					 vector<map<int, double>*>& starting_state_vals,
					 vector<double>& flat_vals,
					 vector<ContextLayer>& context,
					 int& exit_depth,
					 int& exit_node_id,
					 RunHelper& run_helper,
					 ScopeHistory* history) {
	exit_depth = -1;

	if (run_helper.curr_depth > run_helper.max_depth) {
		run_helper.max_depth = run_helper.curr_depth;
	}
	if (run_helper.curr_depth > solution->depth_limit) {
		run_helper.exceeded_depth = true;
		history->exceeded_depth = true;
		return;
	}
	run_helper.curr_depth++;

	if (this->is_loop) {

	} else {
		history->node_histories.push_back(vector<AbstractNodeHistory*>());

		int curr_node_id = starting_node_ids[0];
		starting_node_ids.erase(starting_node_ids.begin());
		if (starting_node_ids.size() > 0) {
			ScopeNode* scope_node = (ScopeNode*)this->nodes[curr_node_id];

			int inner_exit_depth;
			int inner_exit_node_id;

			ScopeNodeHistory* node_history = new ScopeNodeHistory(scope_node);
			history->node_histories[0].push_back(node_history);
			scope_node->halfway_activate(starting_node_ids,
										 starting_state_vals,
										 flat_vals,
										 context,
										 inner_exit_depth,
										 inner_exit_node_id,
										 run_helper,
										 node_history);

			if (inner_exit_depth == -1) {
				curr_node_id = scope_node->next_node_id;
			} else if (inner_exit_depth == 0) {
				curr_node_id = inner_exit_node_id;
			} else {
				exit_depth = inner_exit_depth-1;
				exit_node_id = inner_exit_node_id;
			}
		}

		while (true) {
			if (curr_node_id == -1 || exit_depth != -1) {
				break;
			}

			node_activate_helper(0,
								 curr_node_id,
								 flat_vals,
								 context,
								 exit_depth,
								 exit_node_id,
								 run_helper,
								 history);
		}
	}

	run_helper.curr_depth--;
}

void Scope::node_activate_helper(int iter_index,
								 int& curr_node_id,
								 vector<double>& flat_vals,
								 vector<ContextLayer>& context,
								 int& exit_depth,
								 int& exit_node_id,
								 RunHelper& run_helper,
								 ScopeHistory* history) {
	
}
