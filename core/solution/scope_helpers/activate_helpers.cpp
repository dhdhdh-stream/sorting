#include "scope.h"

#include <iostream>

#include "action_node.h"
#include "branch_experiment.h"
#include "branch_node.h"
#include "branch_stub_node.h"
#include "constants.h"
#include "exit_node.h"
#include "globals.h"
#include "obs_experiment.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void Scope::activate(vector<int>& starting_node_ids,
					 vector<map<int, StateStatus>>& starting_input_state_vals,
					 vector<map<int, StateStatus>>& starting_local_state_vals,
					 Problem& problem,
					 vector<ContextLayer>& context,
					 int& exit_depth,
					 int& exit_node_id,
					 RunHelper& run_helper,
					 ScopeHistory* history) {
	if (run_helper.curr_depth > run_helper.max_depth) {
		run_helper.max_depth = run_helper.curr_depth;
	}
	if (run_helper.curr_depth > solution->depth_limit) {
		run_helper.exceeded_depth = true;
		history->exceeded_depth = true;
		return;
	}
	run_helper.curr_depth++;

	history->node_histories.push_back(vector<AbstractNodeHistory*>());

	int curr_node_id = starting_node_ids[0];
	starting_node_ids.erase(starting_node_ids.begin());
	if (starting_node_ids.size() > 0) {
		context.back().node_id = curr_node_id;

		ScopeNode* scope_node = (ScopeNode*)this->nodes[curr_node_id];
		scope_node->halfway_activate(starting_node_ids,
									 starting_input_state_vals,
									 starting_local_state_vals,
									 curr_node_id,
									 problem,
									 context,
									 exit_depth,
									 exit_node_id,
									 run_helper,
									 history->node_histories[0]);

		context.back().node_id = -1;
	}

	while (true) {
		if (curr_node_id == -1 || exit_depth != -1 || run_helper.exceeded_depth) {
			break;
		}

		node_activate_helper(0,
							 curr_node_id,
							 problem,
							 context,
							 exit_depth,
							 exit_node_id,
							 run_helper,
							 history);
	}

	history->input_state_snapshots = context.back().input_state_vals;
	history->local_state_snapshots = context.back().local_state_vals;

	if (run_helper.phase == RUN_PHASE_UPDATE) {
		run_helper.scope_histories.push_back(history);
		/**
		 * - keep even if early exit, so that can learn good decisions even if early exit
		 */
	} else if (history->inner_branch_experiment_history != NULL) {
		history->experiment_state_snapshots = context.back().experiment_state_vals;
	}

	run_helper.curr_depth--;
}

void Scope::node_activate_helper(int iter_index,
								 int& curr_node_id,
								 Problem& problem,
								 vector<ContextLayer>& context,
								 int& exit_depth,
								 int& exit_node_id,
								 RunHelper& run_helper,
								 ScopeHistory* history) {
	context.back().node_id = curr_node_id;

	if (this->nodes[curr_node_id]->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)this->nodes[curr_node_id];
		action_node->activate(curr_node_id,
							  problem,
							  context,
							  exit_depth,
							  exit_node_id,
							  run_helper,
							  history->node_histories[0]);
	} else if (this->nodes[curr_node_id]->type == NODE_TYPE_SCOPE) {
		ScopeNode* scope_node = (ScopeNode*)this->nodes[curr_node_id];
		scope_node->activate(curr_node_id,
							 problem,
							 context,
							 exit_depth,
							 exit_node_id,
							 run_helper,
							 history->node_histories[0]);
	} else if (this->nodes[curr_node_id]->type == NODE_TYPE_BRANCH) {
		BranchNode* branch_node = (BranchNode*)this->nodes[curr_node_id];
		branch_node->activate(curr_node_id,
							  problem,
							  context,
							  exit_depth,
							  exit_node_id,
							  run_helper,
							  history->node_histories[0]);
	} else if (this->nodes[curr_node_id]->type == NODE_TYPE_BRANCH_STUB) {
		BranchStubNode* branch_stub_node = (BranchStubNode*)this->nodes[curr_node_id];

		branch_stub_node->activate(context);

		curr_node_id = branch_stub_node->next_node_id;
	} else {
		ExitNode* exit_node = (ExitNode*)this->nodes[curr_node_id];

		if (exit_node->exit_depth == 0) {
			curr_node_id = exit_node->exit_node_id;
		} else {
			exit_depth = exit_node->exit_depth-1;
			exit_node_id = exit_node->exit_node_id;
		}
	}

	context.back().node_id = -1;
}
