#include "start_node.h"

#include "run.h"

using namespace std;

void StartNode::step(int& action,
					 bool& is_next,
					 Run* run) {
	run->node_context = this->next_node;
}
