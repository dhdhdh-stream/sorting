#include "branch_node.h"

using namespace std;

BranchNode::BranchNode() {
	this->type = NODE_TYPE_BRANCH;

	this->experiment = NULL:
}

BranchNode::BranchNode(ifstream& input_file,
					   int id) {
	this->type = NODE_TYPE_BRANCH;



}

BranchNode::~BranchNode() {
	if (this->experiment != NULL) {
		delete this->experiment;
	}
}

void BranchNode::save(ofstream& output_file) {
	output_file << this->branch_scope_context.size() << endl;
	for (int c_index = 0; c_index < (int)this->branch_scope_context.size(); c_index++) {
		output_file << this->branch_scope_context[c_index] << endl;
		output_file << this->branch_node_context[c_index] << endl;
	}

	output_file << this->branch_is_pass_through << endl;

	output_file << this->shared_state_is_local.size() << endl;
	for (int s_index = 0; s_index < (int)this->shared_state_is_local.size(); s_index++) {
		output_file << this->shared_state_is_local[s_index] << endl;
		output_file << this->shared_state_indexes[s_index] << endl;
		output_file << this->branch_weights[s_index] << endl;
		output_file << this->original_state_defs[s_index]->id << endl;
	}

	output_file << this->branch_state_is_local.size() << endl;
	for (int s_index = 0; s_index < (int)this->branch_state_is_local.size(); s_index++) {
		output_file << this->branch_state_is_local[s_index] << endl;
		output_file << this->branch_state_indexes[s_index] << endl;
		output_file << this->branch_state_defs[s_index]->id << endl;
	}

	output_file << this->branch_next_node_id << endl;
	output_file << this->original_next_node_id << endl;

	output_file << this->state_defs.size() << endl;
	for (int s_index = 0; s_index < (int)this->state_defs.size(); s_index++) {
		output_file << this->state_is_local[s_index] << endl;
		output_file << this->state_indexes[s_index] << endl;
		output_file << this->state_defs[s_index]->id << endl;
		output_file << this->state_network_indexes[s_index] << endl;
	}

	output_file << this->score_state_defs.size() << endl;
	for (int s_index = 0; s_index < (int)this->score_state_defs.size(); s_index++) {
		output_file << this->score_state_scope_contexts[s_index].size() << endl;
		for (int c_index = 0; c_index < (int)this->score_state_scope_contexts[s_index].size(); c_index++) {
			output_file << this->score_state_scope_contexts[s_index][c_index] << endl;
			output_file << this->score_state_node_contexts[s_index][c_index] << endl;
		}
		output_file << this->score_state_defs[s_index]->id << endl;
		output_file << this->score_state_network_indexes[s_index] << endl;
	}
}

BranchNodeHistory::BranchNodeHistory(BranchNode* node) {

}

BranchNodeHistory::~BranchNodeHistory() {

}
