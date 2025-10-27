#include "solution_helpers.h"

#include "action_node.h"
#include "branch_end_node.h"
#include "branch_node.h"
#include "scope_node.h"
#include "start_node.h"

using namespace std;

AbstractNode* fetch_path_end(AbstractNode* node_context) {
	set<BranchNode*> branch_nodes_seen;

	AbstractNode* prev_node = NULL;
	AbstractNode* curr_node = node_context;
	while (true) {
		switch (curr_node->type) {
		case NODE_TYPE_START:
			{
				StartNode* node = (StartNode*)curr_node;
				prev_node = curr_node;
				curr_node = node->next_node;
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* node = (ActionNode*)curr_node;
				prev_node = curr_node;
				curr_node = node->next_node;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* node = (ScopeNode*)curr_node;
				prev_node = curr_node;
				curr_node = node->next_node;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* node = (BranchNode*)curr_node;

				branch_nodes_seen.insert(node);

				prev_node = curr_node;
				/**
				 * - simply always choose original
				 */
				curr_node = node->original_next_node;
			}
			break;
		case NODE_TYPE_BRANCH_END:
			{
				BranchEndNode* node = (BranchEndNode*)curr_node;

				if (node != node_context) {
					set<BranchNode*>::iterator it = branch_nodes_seen.find(node->branch_start_node);
					if (it == branch_nodes_seen.end()) {
						return prev_node;
					} else {
						branch_nodes_seen.erase(it);
					}
				}

				prev_node = curr_node;
				curr_node = node->next_node;
			}
			break;
		}
	}
}
