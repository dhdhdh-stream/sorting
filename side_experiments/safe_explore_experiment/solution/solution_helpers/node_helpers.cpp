#include "solution_helpers.h"

#include "action_node.h"
#include "branch_node.h"
#include "obs_node.h"
#include "scope_node.h"
#include "start_node.h"

using namespace std;

void gather_all_children_helper(AbstractNode* curr_node,
								set<AbstractNode*>& children) {
	if (curr_node == NULL
			|| children.find(curr_node) != children.end()) {
		// do nothing
	} else {
		children.insert(curr_node);

		switch (curr_node->type) {
		case NODE_TYPE_START:
			{
				StartNode* start_node = (StartNode*)curr_node;
				gather_all_children_helper(start_node->next_node,
										   children);
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)curr_node;
				gather_all_children_helper(action_node->next_node,
										   children);
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)curr_node;
				gather_all_children_helper(scope_node->next_node,
										   children);
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)curr_node;
				gather_all_children_helper(branch_node->original_next_node,
										   children);
				gather_all_children_helper(branch_node->branch_next_node,
										   children);
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)curr_node;
				gather_all_children_helper(obs_node->next_node,
										   children);
			}
			break;
		}
	}
}
