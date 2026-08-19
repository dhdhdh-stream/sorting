#include "scope_node.h"

#include "predict_network.h"

using namespace std;

void ScopeNode::predict_step(Eigen::VectorXf& state,
							 AbstractNode*& node_context) {
	this->predict_network->activate(state);

	node_context = this->next_node;
}
