#include "scope.h"

#include "action_node.h"
#include "branch_node.h"
#include "branch_stub_node.h"
#include "globals.h"
#include "exit_node.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void Scope::create_sequence_activate(vector<int>& starting_node_ids,
									 vector<map<int, StateStatus>>& starting_input_state_vals,
									 vector<map<int, StateStatus>>& starting_local_state_vals,
									 vector<map<pair<bool,int>, int>>& starting_state_mappings,
									 Problem& problem,
									 vector<ContextLayer>& context,
									 int target_num_nodes,
									 int& curr_num_nodes,
									 Sequence* new_sequence,
									 vector<map<pair<bool,int>, int>>& state_mappings,
									 int& new_num_input_states,
									 vector<AbstractNode*>& new_nodes,
									 RunHelper& run_helper) {
	if (run_helper.curr_depth > solution->depth_limit) {
		run_helper.exceeded_depth = true;
		return;
	}
	run_helper.curr_depth++;

	// this->is_loop == false

	int curr_node_id = starting_node_ids[0];
	starting_node_ids.erase(starting_node_ids.begin());
	if (starting_node_ids.size() > 0) {
		ScopeNode* scope_node = (ScopeNode*)this->nodes[curr_node_id];

		scope_node->halfway_create_sequence_activate(
			starting_node_ids,
			starting_input_state_vals,
			starting_local_state_vals,
			starting_state_mappings,
			problem,
			context,
			target_num_nodes,
			curr_num_nodes,
			new_sequence,
			state_mappings,
			new_num_input_states,
			new_nodes,
			run_helper);

		curr_node_id = scope_node->next_node_id;
	}

	while (true) {
		if (curr_num_nodes == target_num_nodes
				|| curr_node_id == -1
				|| run_helper.exceeded_depth) {
			break;
		}

		node_create_sequence_activate_helper(curr_node_id,
											 problem,
											 context,
											 target_num_nodes,
											 curr_num_nodes,
											 new_sequence,
											 state_mappings,
											 new_num_input_states,
											 new_nodes,
											 run_helper);
	}

	run_helper.curr_depth--;
}

void Scope::node_create_sequence_activate_helper(
		int& curr_node_id,
		Problem& problem,
		vector<ContextLayer>& context,
		int target_num_nodes,
		int& curr_num_nodes,
		Sequence* new_sequence,
		vector<map<pair<bool,int>, int>>& state_mappings,
		int& new_num_input_states,
		vector<AbstractNode*>& new_nodes,
		RunHelper& run_helper) {
	// no need to set context.back().node_id

	if (this->nodes[curr_node_id]->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)this->nodes[curr_node_id];

		action_node->create_sequence_activate(problem,
											  context,
											  target_num_nodes,
											  curr_num_nodes,
											  new_sequence,
											  state_mappings,
											  new_num_input_states,
											  new_nodes);

		curr_node_id = action_node->next_node_id;
	} else if (this->nodes[curr_node_id]->type == NODE_TYPE_SCOPE) {
		ScopeNode* scope_node = (ScopeNode*)this->nodes[curr_node_id];

		scope_node->create_sequence_activate(problem,
											 context,
											 target_num_nodes,
											 curr_num_nodes,
											 new_sequence,
											 state_mappings,
											 new_num_input_states,
											 new_nodes,
											 run_helper);

		curr_node_id = scope_node->next_node_id;
	} else if (this->nodes[curr_node_id]->type == NODE_TYPE_BRANCH) {
		BranchNode* branch_node = (BranchNode*)this->nodes[curr_node_id];

		bool is_branch;
		branch_node->create_sequence_activate(is_branch,
											  context,
											  target_num_nodes,
											  curr_num_nodes,
											  new_sequence,
											  state_mappings,
											  new_num_input_states,
											  new_nodes,
											  run_helper);

		if (is_branch) {
			curr_node_id = branch_node->branch_next_node_id;
		} else {
			curr_node_id = branch_node->original_next_node_id;
		}
	} else if (this->nodes[curr_node_id]->type == NODE_TYPE_BRANCH_STUB) {
		BranchStubNode* branch_stub_node = (BranchStubNode*)this->nodes[curr_node_id];

		branch_stub_node->create_sequence_activate(context,
												   target_num_nodes,
												   curr_num_nodes,
												   new_sequence,
												   state_mappings,
												   new_num_input_states,
												   new_nodes);

		curr_node_id = branch_stub_node->next_node_id;
	} else {
		// this->nodes[curr_node_id]->type == NODE_TYPE_EXIT

		curr_node_id = -1;
		// simply set to -1 to signal exit
	}

	// no need to set context.back().node_id
}
