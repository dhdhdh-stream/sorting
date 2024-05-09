#include "new_action_helpers.h"

#include "action_node.h"
#include "branch_node.h"
#include "info_branch_node.h"
#include "new_action_tracker.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void add_new_action(Solution* parent_solution) {
	ActionNode* new_ending_node = NULL;

	for (map<AbstractNode*, NewActionNodeTracker*>::iterator it = parent_solution->new_action_tracker->node_trackers.begin();
			it != parent_solution->new_action_tracker->node_trackers.end(); it++) {
		AbstractNode* duplicate_end_node;
		if (it->second->exit_next_node == NULL) {
			if (new_ending_node == NULL) {
				new_ending_node = new ActionNode();
				new_ending_node->parent = parent_solution->current;
				new_ending_node->id = parent_solution->current->node_counter;
				parent_solution->current->node_counter++;
				parent_solution->current->nodes[new_ending_node->id] = new_ending_node;

				new_ending_node->action = Action(ACTION_NOOP);

				new_ending_node->next_node_id = -1;
				new_ending_node->next_node = NULL;
			}

			duplicate_end_node = new_ending_node;
		} else {
			duplicate_end_node = parent_solution->current->nodes[it->second->exit_next_node->id];
		}

		ScopeNode* new_scope_node = new ScopeNode();
		new_scope_node->parent = parent_solution->current;
		new_scope_node->id = parent_solution->current->node_counter;
		parent_solution->current->node_counter++;
		parent_solution->current->nodes[new_scope_node->id] = new_scope_node;

		new_scope_node->scope = parent_solution->scopes.back();

		new_scope_node->next_node_id = duplicate_end_node->id;
		new_scope_node->next_node = duplicate_end_node;

		AbstractNode* duplicate_start_node = parent_solution->current->nodes[it->first->id];
		switch (duplicate_start_node->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)duplicate_start_node;

				action_node->next_node_id = new_scope_node->id;
				action_node->next_node = new_scope_node;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)duplicate_start_node;

				scope_node->next_node_id = new_scope_node->id;
				scope_node->next_node = new_scope_node;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)duplicate_start_node;

				if (it->second->is_branch) {
					branch_node->branch_next_node_id = new_scope_node->id;
					branch_node->branch_next_node = new_scope_node;
				} else {
					branch_node->original_next_node_id = new_scope_node->id;
					branch_node->original_next_node = new_scope_node;
				}
			}
			break;
		case NODE_TYPE_INFO_BRANCH:
			{
				InfoBranchNode* info_branch_node = (InfoBranchNode*)duplicate_start_node;

				if (it->second->is_branch) {
					info_branch_node->branch_next_node_id = new_scope_node->id;
					info_branch_node->branch_next_node = new_scope_node;
				} else {
					info_branch_node->original_next_node_id = new_scope_node->id;
					info_branch_node->original_next_node = new_scope_node;
				}
			}
			break;
		}
	}
}
