#include "solution_helpers.h"

#include <set>

#include "action_node.h"
#include "branch_node.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "start_node.h"

using namespace std;

bool check_can_protect(AbstractNode* starting_node,
					   AbstractNode* ending_node) {
	Scope* scope = starting_node->parent;

	set<AbstractNode*> ancestors;
	vector<AbstractNode*> ancestors_to_check;
	ancestors.insert(ending_node);
	if (ending_node == NULL) {
		for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
				it != scope->nodes.end(); it++) {
			if (it->second->type == NODE_TYPE_OBS) {
				ObsNode* obs_node = (ObsNode*)it->second;
				if (obs_node->next_node == NULL) {
					ancestors.insert(obs_node);
					ancestors_to_check.push_back(obs_node);
					break;
				}
			}
		}
	} else {
		ancestors_to_check.push_back(ending_node);
	}
	while (ancestors_to_check.size() > 0) {
		AbstractNode* curr_node = ancestors_to_check.back();
		ancestors_to_check.pop_back();
		if (curr_node == starting_node) {
			// do nothing
		} else if (curr_node->ancestor_ids.size() == 0) {
			return false;
		} else {
			for (int a_index = 0; a_index < (int)curr_node->ancestor_ids.size(); a_index++) {
				AbstractNode* node = scope->nodes[curr_node->ancestor_ids[a_index]];
				if (ancestors.find(node) == ancestors.end()) {
					ancestors.insert(node);
					ancestors_to_check.push_back(node);
				}
			}
		}
	}

	set<AbstractNode*> children;
	vector<AbstractNode*> children_to_check;
	children.insert(starting_node);
	children_to_check.push_back(starting_node);
	while (children_to_check.size() > 0) {
		AbstractNode* curr_node = children_to_check.back();
		children_to_check.pop_back();
		if (curr_node == ending_node) {
			// do nothing
		} else if (curr_node == NULL) {
			return false;
		} else {
			switch (curr_node->type) {
			case NODE_TYPE_START:
				{
					StartNode* start_node = (StartNode*)curr_node;
					if (children.find(start_node->next_node) == children.end()) {
						children.insert(start_node->next_node);
						children_to_check.push_back(start_node->next_node);
					}
				}
				break;
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)curr_node;
					if (children.find(action_node->next_node) == children.end()) {
						children.insert(action_node->next_node);
						children_to_check.push_back(action_node->next_node);
					}
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)curr_node;
					if (children.find(scope_node->next_node) == children.end()) {
						children.insert(scope_node->next_node);
						children_to_check.push_back(scope_node->next_node);
					}
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)curr_node;
					if (children.find(branch_node->original_next_node) == children.end()) {
						children.insert(branch_node->original_next_node);
						children_to_check.push_back(branch_node->original_next_node);
					}
					if (children.find(branch_node->branch_next_node) == children.end()) {
						children.insert(branch_node->branch_next_node);
						children_to_check.push_back(branch_node->branch_next_node);
					}
				}
				break;
			case NODE_TYPE_OBS:
				{
					ObsNode* obs_node = (ObsNode*)curr_node;
					if (children.find(obs_node->next_node) == children.end()) {
						children.insert(obs_node->next_node);
						children_to_check.push_back(obs_node->next_node);
					}
				}
				break;
			}
		}
	}

	return true;
}
