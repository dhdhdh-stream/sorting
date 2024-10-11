#include "scope.h"

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "scope_node.h"

using namespace std;

void node_align_activate_helper(AbstractNode*& curr_node,
								Alignment& alignment,
								vector<ContextLayer>& context) {
	switch (curr_node->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* node = (ActionNode*)curr_node;
			node->align_activate(curr_node,
								 alignment,
								 context);
		}

		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* node = (ScopeNode*)curr_node;
			node->align_activate(curr_node,
								 alignment,
								 context);
		}

		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* node = (BranchNode*)curr_node;
			node->align_activate(curr_node,
								 context);
		}

		break;
	}
}

void Scope::align_activate(Alignment& alignment,
						   vector<ContextLayer>& context) {
	context.push_back(ContextLayer());

	context.back().scope_id = this->id;

	AbstractNode* curr_node = this->nodes[0];
	while (true) {
		if (curr_node == NULL) {
			break;
		}

		node_align_activate_helper(curr_node,
								   alignment,
								   context);
	}

	context.pop_back();
}
