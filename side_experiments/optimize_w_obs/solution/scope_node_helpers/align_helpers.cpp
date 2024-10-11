#include "scope_node.h"

#include "scope.h"

using namespace std;

void ScopeNode::align_activate(AbstractNode*& curr_node,
							   Alignment& alignment,
							   vector<ContextLayer>& context) {
	context.back().node_id = this->id;

	this->scope->align_activate(alignment,
								context);

	curr_node = this->next_node;
}
