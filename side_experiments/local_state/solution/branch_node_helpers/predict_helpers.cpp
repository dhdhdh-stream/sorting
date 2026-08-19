#include "branch_node.h"

#include "score_network.h"
#include "utilities.h"

using namespace std;

void BranchNode::predict_step(Eigen::VectorXf& state,
							  AbstractNode*& node_context) {
	if (this->consec_original >= CONSEC_DEPRECATE_LIMIT) {
		node_context = this->original_next_node;
		return;
	}
	if (this->consec_branch >= CONSEC_DEPRECATE_LIMIT) {
		node_context = this->branch_next_node;
		return;
	}

	bool is_branch;
	this->original_network->activate(state);
	this->branch_network->activate(state);
	if (this->branch_network->output->acti_vals(0) >= this->original_network->output->acti_vals(0)) {
		is_branch = true;
	} else {
		is_branch = false;
	}

	if (is_branch) {
		node_context = this->branch_next_node;
	} else {
		node_context = this->original_next_node;
	}
}
