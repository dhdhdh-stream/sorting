#include "action_node.h"

#include "run.h"
#include "world_model_helpers.h"

using namespace std;

void ActionNode::step(int& action,
					  bool& is_next,
					  Run* run) {
	action_helper(this->action,
				  run->state,
				  run->wrapper);

	action = this->action;
	is_next = true;

	run->node_context = this->next_node;
}
