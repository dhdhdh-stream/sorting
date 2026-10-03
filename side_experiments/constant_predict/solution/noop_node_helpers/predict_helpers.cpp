#include "noop_node.h"

using namespace std;

void NoopNode::predict_step(Eigen::VectorXf& state,
							AbstractNode*& node_context) {
	node_context = this->next_node;
}
