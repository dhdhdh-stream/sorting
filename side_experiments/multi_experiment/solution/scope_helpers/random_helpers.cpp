#include "scope.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "obs_node.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void Scope::random_continue(AbstractNode* starting_node,
							int num_following,
							set<AbstractNode*>& potential_included_nodes) {
	AbstractNode* curr_node = starting_node;
	for (int f_index = 0; f_index < num_following; f_index++) {
		if (curr_node == NULL) {
			break;
		}

		switch (curr_node->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* node = (ActionNode*)curr_node;

				potential_included_nodes.insert(curr_node);

				curr_node = node->next_node;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* node = (ScopeNode*)curr_node;

				potential_included_nodes.insert(curr_node);

				curr_node = node->next_node;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* node = (BranchNode*)curr_node;

				potential_included_nodes.insert(curr_node);

				uniform_int_distribution<int> distribution(0, 1);
				if (distribution(generator) == 0) {
					curr_node = node->branch_next_node;
				} else {
					curr_node = node->original_next_node;
				}
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* node = (ObsNode*)curr_node;

				potential_included_nodes.insert(curr_node);

				curr_node = node->next_node;
			}
			break;
		}
	}
}
