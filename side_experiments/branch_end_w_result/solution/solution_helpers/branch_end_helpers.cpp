#include "solution_helpers.h"

#include "action_node.h"
#include "branch_node.h"
#include "obs_node.h"
#include "scope_node.h"
#include "start_node.h"

using namespace std;

AbstractNode* fetch_path_end(AbstractNode* node_context) {
	set<int> branch_node_ids_seen;

	AbstractNode* curr_node = node_context;
	while (true) {
		switch (curr_node->type) {
		case NODE_TYPE_START:
			{
				StartNode* node = (StartNode*)curr_node;
				curr_node = node->next_node;
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* node = (ActionNode*)curr_node;
				curr_node = node->next_node;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* node = (ScopeNode*)curr_node;
				curr_node = node->next_node;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* node = (BranchNode*)curr_node;

				branch_node_ids_seen.insert(node->id);

				/**
				 * - simply always choose original
				 */
				curr_node = node->original_next_node;
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* node = (ObsNode*)curr_node;

				if (node->branch_start_node_id != -1) {
					set<int>::iterator it = branch_node_ids_seen.find(node->branch_start_node_id);
					if (it == branch_node_ids_seen.end()) {
						return curr_node;
					} else {
						branch_node_ids_seen.erase(it);
					}
				}

				curr_node = node->next_node;
			}
			break;
		}
	}
}
