#include "solution_helpers.h"

#include "action_node.h"
#include "branch_node.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

void gather_possible_exits_helper(int l_index,
								  vector<pair<int,AbstractNode*>>& possible_exits,
								  vector<Scope*>& experiment_scope_context,
								  vector<AbstractNode*>& experiment_node_context,
								  bool experiment_is_branch,
								  int& exit_depth,
								  AbstractNode*& exit_node,
								  int& random_curr_depth,
								  int& random_throw_id,
								  bool& random_exceeded_limit) {
	int inner_exit_depth = -1;
	AbstractNode* inner_exit_node = NULL;
	if (l_index < (int)experiment_scope_context.size()-1) {
		gather_possible_exits_helper(l_index+1,
									 possible_exits,
									 experiment_scope_context,
									 experiment_node_context,
									 experiment_is_branch,
									 inner_exit_depth,
									 inner_exit_node,
									 random_curr_depth,
									 random_throw_id,
									 random_exceeded_limit);
	}

	if (random_exceeded_limit) {
		// do nothing
	} else if (random_throw_id != -1) {
		ScopeNode* scope_node = (ScopeNode*)experiment_node_context[l_index];
		map<int, AbstractNode*>::iterator it = scope_node->catches.find(random_throw_id);
		if (it != scope_node->catches.end()) {
			random_throw_id = -1;

			vector<Scope*> scope_context(experiment_scope_context.begin(), experiment_scope_context.begin() + l_index+1);
			vector<AbstractNode*> node_context(experiment_node_context.begin(), experiment_node_context.begin() + l_index+1);

			experiment_scope_context[l_index]->random_exit_activate(
				it->second,
				scope_context,
				node_context,
				exit_depth,
				exit_node,
				random_curr_depth,
				random_throw_id,
				random_exceeded_limit,
				experiment_scope_context.size()-1 - l_index,
				possible_exits);
		} else {
			// do nothing
		}
	} else if (inner_exit_depth == -1) {
		AbstractNode* starting_node;
		AbstractNode* experiment_node = experiment_node_context[l_index];
		if (experiment_node->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)experiment_node;
			starting_node = action_node->next_node;
		} else if (experiment_node->type == NODE_TYPE_SCOPE) {
			ScopeNode* scope_node = (ScopeNode*)experiment_node;
			starting_node = scope_node->next_node;
		} else {
			BranchNode* branch_node = (BranchNode*)experiment_node;
			if (experiment_is_branch) {
				starting_node = branch_node->branch_next_node;
			} else {
				starting_node = branch_node->original_next_node;
			}
		}

		vector<Scope*> scope_context(experiment_scope_context.begin(), experiment_scope_context.begin() + l_index+1);
		vector<AbstractNode*> node_context(experiment_node_context.begin(), experiment_node_context.begin() + l_index+1);

		experiment_scope_context[l_index]->random_exit_activate(
			starting_node,
			scope_context,
			node_context,
			exit_depth,
			exit_node,
			random_curr_depth,
			random_throw_id,
			random_exceeded_limit,
			experiment_scope_context.size()-1 - l_index,
			possible_exits);
	} else if (inner_exit_depth == 0) {
		vector<Scope*> scope_context(experiment_scope_context.begin(), experiment_scope_context.begin() + l_index+1);
		vector<AbstractNode*> node_context(experiment_node_context.begin(), experiment_node_context.begin() + l_index+1);

		experiment_scope_context[l_index]->random_exit_activate(
			inner_exit_node,
			scope_context,
			node_context,
			exit_depth,
			exit_node,
			random_curr_depth,
			random_throw_id,
			random_exceeded_limit,
			experiment_scope_context.size()-1 - l_index,
			possible_exits);
	} else {
		exit_depth = inner_exit_depth-1;
		exit_node = inner_exit_node;
	}
}

void gather_possible_exits(vector<pair<int,AbstractNode*>>& possible_exits,
						   vector<Scope*>& experiment_scope_context,
						   vector<AbstractNode*>& experiment_node_context,
						   bool experiment_is_branch,
						   RunHelper& run_helper) {
	// unused
	int exit_depth = -1;
	AbstractNode* exit_node = NULL;

	int random_curr_depth = run_helper.curr_depth;
	int random_throw_id = -1;
	bool random_exceeded_limit = false;

	gather_possible_exits_helper(0,
								 possible_exits,
								 experiment_scope_context,
								 experiment_node_context,
								 experiment_is_branch,
								 exit_depth,
								 exit_node,
								 random_curr_depth,
								 random_throw_id,
								 random_exceeded_limit);
}
