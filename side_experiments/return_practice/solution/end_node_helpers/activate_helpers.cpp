#include "end_node.h"

#include "run.h"

using namespace std;

void EndNode::step(int& action,
				   bool& is_next,
				   Run* run) {
	run->node_context = NULL;
}
