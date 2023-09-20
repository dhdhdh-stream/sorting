#include "scope.h"

using namespace std;

void Scope::create_sequence_activate(vector<int>& starting_node_ids,
									 vector<map<int, double>*>& starting_state_vals,
									 vector<double>& flat_vals,
									 vector<ContextLayer>& context,
									 int target_num_nodes,
									 int& curr_num_nodes,
									 map<int, map<int, int>>& state_mapping,
									 int& new_num_states,
									 vector<AbstractNode*>& new_nodes,
									 RunHelper& run_helper) {
	/**
	 * - OK if normal exit, but won't be early exit
	 *   - if normal exit, then continue search for target_num_nodes outside
	 */

	run_helper.curr_depth++;

	// this->is_loop == false

	int curr_node_id = starting_node_ids[0];
	starting_node_ids.erase(starting_node_ids.begin());
	if (starting_node_ids.size() > 0) {
		ScopeNode* scope_node = (ScopeNode*)this->nodes[curr_node_id];

		scope_node->halfway_create_sequence_activate(
			starting_node_ids,
			starting_state_vals,
			flat_vals,
			context,
			target_num_nodes,
			curr_num_nodes,
			state_mapping,
			new_num_states,
			new_nodes,
			run_helper);

		curr_node_id = scope_node->next_node_id;
	}

	while (true) {
		if (curr_num_nodes == target_num_nodes
				|| curr_node_id == -1) {
			break;
		}

		node_create_sequence_activate_helper(curr_node_id,
											 flat_vals,
											 context,
											 target_num_nodes,
											 curr_num_nodes,
											 state_mapping,
											 new_num_states,
											 new_nodes,
											 run_helper);
	}

	run_helper.curr_depth--;
}

void Scope::node_create_sequence_activate_helper(
		int& curr_node_id,
		vector<double>& flat_vals,
		vector<ContextLayer>& context,
		int target_num_nodes,
		int& curr_num_nodes,
		map<int, map<int, int>>& state_mapping,
		int& new_num_states,
		vector<AbstractNode*>& new_nodes,
		RunHelper& run_helper) {
	if (this->nodes[curr_node_id]->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)this->nodes[curr_node_id];

		action_node->create_sequence_activate(flat_vals,
											  context,
											  target_num_nodes,
											  curr_num_nodes,
											  state_mapping,
											  new_num_states,
											  new_nodes);

		curr_node_id = action_node->next_node_id;
	} else if (this->nodes[curr_node_id]->type == NODE_TYPE_SCOPE) {
		ScopeNode* scope_node = (ScopeNode*)this->nodes[curr_node_id];

		scope_node->create_sequence_activate(flat_vals,
											 context,
											 target_num_nodes,
											 curr_num_nodes,
											 state_mapping,
											 new_num_states,
											 new_nodes,
											 run_helper);

		curr_node_id = scope_node->next_node_id;
	} else if (this->nodes[curr_node_id]->type == NODE_TYPE_BRANCH) {
		BranchNode* branch_node = (BranchNode*)this->nodes[curr_node_id];

		bool is_branch;
		branch_node->create_sequence_activate(context,
											  target_num_nodes,
											  curr_num_nodes,
											  state_mapping,
											  new_num_states,
											  new_nodes,
											  is_branch);

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
												   state_mapping,
												   new_num_states,
												   new_nodes);

		curr_node_id = branch_stub_node->next_node_id;
	} else {
		ExitNode* exit_node = (ExitNode*)this->nodes[curr_node_id];

		if (exit_node->exit_depth == 0) {
			curr_node_id = exit_node->exit_node_id;
		} else {
			exit_depth = exit_node->exit_depth;
			exit_node_id = exit_node->exit_node_id;
		}
	}
}
