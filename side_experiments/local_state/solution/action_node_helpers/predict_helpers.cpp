#include "action_node.h"

#include <iostream>

#include "action_network.h"
#include "predict_network.h"

using namespace std;

void ActionNode::predict_step(Eigen::VectorXf& state,
							  AbstractNode*& node_context) {
	this->action_network->activate(state);

	this->predict_network->activate(state);

	if (!this->is_generic) {
		node_context = this->next_node;
	}
}
