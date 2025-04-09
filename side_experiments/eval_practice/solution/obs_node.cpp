#include "obs_node.h"

using namespace std;

ObsNode::ObsNode() {
	this->type = NODE_TYPE_OBS;
}

ObsNodeHistory::ObsNodeHistory(ObsNode* node) {
	this->node = node;
}
