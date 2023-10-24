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
#include "state_network.h"

using namespace std;

void Scope::view_activate(vector<int>& starting_node_ids,
						  vector<map<int, StateStatus>>& starting_input_state_vals,
						  vector<map<int, StateStatus>>& starting_local_state_vals,
						  Problem& problem,
						  vector<ContextLayer>& context,
						  int& exit_depth,
						  int& exit_node_id,
						  RunHelper& run_helper) {
	cout << "scope #" << this->id << endl;
	cout << endl;

	if (run_helper.curr_depth > run_helper.max_depth) {
		run_helper.max_depth = run_helper.curr_depth;
	}
	if (run_helper.curr_depth > solution->depth_limit) {
		run_helper.exceeded_depth = true;
		cout << "exceeded_depth" << endl;
		return;
	}
	run_helper.curr_depth++;

	int curr_node_id = starting_node_ids[0];
	starting_node_ids.erase(starting_node_ids.begin());
	if (starting_node_ids.size() > 0) {
		context.back().node_id = curr_node_id;

		ScopeNode* scope_node = (ScopeNode*)this->nodes[curr_node_id];
		scope_node->halfway_view_activate(starting_node_ids,
										  starting_input_state_vals,
										  starting_local_state_vals,
										  curr_node_id,
										  problem,
										  context,
										  exit_depth,
										  exit_node_id,
										  run_helper);

		context.back().node_id = -1;
	}

	while (true) {
		if (curr_node_id == -1 || exit_depth != -1 || run_helper.exceeded_depth) {
			break;
		}

		node_view_activate_helper(0,
								  curr_node_id,
								  problem,
								  context,
								  exit_depth,
								  exit_node_id,
								  run_helper);
	}

	{
		double predicted_score = this->average_score;

		for (map<int, StateStatus>::iterator it = context.back().input_state_vals.begin();
				it != context.back().input_state_vals.end(); it++) {
			StateNetwork* last_network = it->second.last_network;
			if (last_network != NULL) {
				double normalized = (it->second.val - last_network->ending_mean)
					/ last_network->ending_standard_deviation;
				predicted_score += this->input_state_weights[it->first] * normalized;
			} else {
				predicted_score += this->input_state_weights[it->first] * it->second.val;
			}
		}

		for (map<int, StateStatus>::iterator it = context.back().local_state_vals.begin();
				it != context.back().local_state_vals.end(); it++) {
			StateNetwork* last_network = it->second.last_network;
			if (last_network != NULL) {
				double normalized = (it->second.val - last_network->ending_mean)
					/ last_network->ending_standard_deviation;
				predicted_score += this->local_state_weights[it->first] * normalized;
			} else {
				predicted_score += this->local_state_weights[it->first] * it->second.val;
			}
		}

		cout << "predicted_score: " << predicted_score << endl;
	}

	cout << endl;

	run_helper.curr_depth--;
}

void Scope::node_view_activate_helper(int iter_index,
									  int& curr_node_id,
									  Problem& problem,
									  vector<ContextLayer>& context,
									  int& exit_depth,
									  int& exit_node_id,
									  RunHelper& run_helper) {
	context.back().node_id = curr_node_id;

	if (this->nodes[curr_node_id]->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)this->nodes[curr_node_id];
		action_node->view_activate(curr_node_id,
								   problem,
								   context,
								   exit_depth,
								   exit_node_id,
								   run_helper);
	} else if (this->nodes[curr_node_id]->type == NODE_TYPE_SCOPE) {
		ScopeNode* scope_node = (ScopeNode*)this->nodes[curr_node_id];
		scope_node->view_activate(curr_node_id,
								  problem,
								  context,
								  exit_depth,
								  exit_node_id,
								  run_helper);
	} else if (this->nodes[curr_node_id]->type == NODE_TYPE_BRANCH) {
		BranchNode* branch_node = (BranchNode*)this->nodes[curr_node_id];
		branch_node->view_activate(curr_node_id,
								   problem,
								   context,
								   exit_depth,
								   exit_node_id,
								   run_helper);
	} else if (this->nodes[curr_node_id]->type == NODE_TYPE_BRANCH_STUB) {
		BranchStubNode* branch_stub_node = (BranchStubNode*)this->nodes[curr_node_id];

		branch_stub_node->view_activate(context);

		curr_node_id = branch_stub_node->next_node_id;
	} else {
		ExitNode* exit_node = (ExitNode*)this->nodes[curr_node_id];

		cout << "exit node #" << curr_node_id << endl;

		cout << "exit_depth: " << exit_node->exit_depth << endl;
		cout << "exit_node_id: " << exit_node->exit_node_id << endl;

		cout << endl;

		if (exit_node->exit_depth == 0) {
			curr_node_id = exit_node->exit_node_id;
		} else {
			exit_depth = exit_node->exit_depth-1;
			exit_node_id = exit_node->exit_node_id;
		}
	}

	context.back().node_id = -1;
}
