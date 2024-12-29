#include "branch_node.h"

#include "abstract_experiment.h"
#include "globals.h"
#include "scope.h"
#include "solution.h"

using namespace std;

BranchNode::BranchNode() {
	this->type = NODE_TYPE_BRANCH;
}

BranchNode::BranchNode(BranchNode* original) {
	this->type = NODE_TYPE_BRANCH;

	this->factor_ids = original->factor_ids;
	this->factor_weights = original->factor_weights;

	this->original_next_node_id = original->original_next_node_id;
	this->branch_next_node_id = original->branch_next_node_id;

	this->ancestor_ids = original->ancestor_ids;

	this->average_instances_per_run = 0.0;

	this->was_commit = false;

	this->num_measure = 0;
	this->sum_score = 0.0;

	#if defined(MDEBUG) && MDEBUG
	this->verify_key = NULL;
	#endif /* MDEBUG */
}

BranchNode::~BranchNode() {
	for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
		this->experiments[e_index]->decrement(this);
	}
}

void BranchNode::clean_inputs(int node_id) {
	for (int f_index = (int)this->factor_ids.size()-1; f_index >= 0; f_index--) {
		if (this->factor_ids[f_index].first == node_id) {
			
		}
	}
}

// TODO: initiate adding all the inputs
