#include "scope.h"

#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "exit_node.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void node_create_sequence_activate_helper(AbstractNode*& curr_node,
										  Problem& problem,
										  vector<ContextLayer>& context,
										  int target_num_nodes,
										  int& curr_num_nodes,
										  Sequence* new_sequence,
										  vector<map<pair<bool,int>, int>>& state_mappings,
										  int& new_num_input_states,
										  vector<AbstractNode*>& new_nodes,
										  RunHelper& run_helper) {
	if (curr_node->type == NODE_TYPE_ACTION) {
		ActionNode* node = (ActionNode*)curr_node;

		node->create_sequence_activate(problem,
									   context,
									   target_num_nodes,
									   curr_num_nodes,
									   new_sequence,
									   state_mappings,
									   new_num_input_states,
									   new_nodes);

		curr_node = node->next_node;
	} else if (curr_node->type == NODE_TYPE_SCOPE) {
		ScopeNode* node = (ScopeNode*)curr_node;

		node->create_sequence_activate(problem,
									   context,
									   target_num_nodes,
									   curr_num_nodes,
									   new_sequence,
									   state_mappings,
									   new_num_input_states,
									   new_nodes,
									   run_helper);

		curr_node = node->next_node;
	} else if (curr_node->type == NODE_TYPE_BRANCH) {
		BranchNode* node = (BranchNode*)curr_node;

		bool is_branch;
		node->activate(is_branch,
					   context);

		if (is_branch) {
			curr_node = node->branch_next_node;
		} else {
			curr_node = node->original_next_node;
		}
	} else {
		// curr_node->type == NODE_TYPE_EXIT

		curr_node = NULL;
		// simply set to NULL to signal exit
	}
}

void Scope::create_sequence_activate(vector<AbstractNode*>& starting_nodes,
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

	AbstractNode* curr_node = starting_nodes[0];
	starting_nodes.erase(starting_nodes.begin());
	if (starting_nodes.size() > 0) {
		ScopeNode* scope_node = (ScopeNode*)curr_node;

		scope_node->halfway_create_sequence_activate(
			starting_nodes,
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

		curr_node = scope_node->next_node;
	}

	while (true) {
		if (curr_num_nodes == target_num_nodes
				|| curr_node == NULL
				|| run_helper.exceeded_depth) {
			break;
		}

		node_create_sequence_activate_helper(curr_node,
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
