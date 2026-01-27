#include "logic_helpers.h"

#include "logic_tree.h"
#include "split_node.h"

using namespace std;

void find_parent(AbstractLogicNode* node,
				 LogicTree* logic_tree,
				 SplitNode*& parent,
				 bool& is_branch) {
	for (map<int, AbstractLogicNode*>::iterator it = logic_tree->nodes.begin();
			it != logic_tree->nodes.end(); it++) {
		switch (it->second->type) {
		case LOGIC_NODE_TYPE_SPLIT:
			{
				SplitNode* split_node = (SplitNode*)it->second;
				if (split_node->original_node == node) {
					parent = split_node;
					is_branch = false;
					return;
				} else if (split_node->branch_node == node) {
					parent = split_node;
					is_branch = true;
					return;
				}
			}
			break;
		}
	}
}

void update_weight_helper(AbstractLogicNode* node,
						  double multiplier) {
	node->weight *= multiplier;

	switch (node->type) {
	case LOGIC_NODE_TYPE_SPLIT:
		{
			SplitNode* split_node = (SplitNode*)node;
			update_weight_helper(split_node->original_node,
								 multiplier);
			update_weight_helper(split_node->branch_node,
								 multiplier);
		}
		break;
	}
}
