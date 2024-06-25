#include "branch_end_node.h"

#include "abstract_experiment.h"
#include "abstract_scope.h"

using namespace std;

BranchEndNode::BranchEndNode() {
	this->type = NODE_TYPE_BRANCH_END;
}

BranchEndNode::BranchEndNode(BranchEndNode* original) {
	this->type = NODE_TYPE_BRANCH_END;

	this->branch_start_node_id = original->branch_start_node_id;

	this->next_node_id = original->next_node_id;
}

BranchEndNode::~BranchEndNode() {
	for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
		this->experiments[e_index]->decrement(this);
	}
}

void BranchEndNode::save(ofstream& output_file) {
	output_file << this->branch_start_node_id << endl;

	output_file << this->next_node_id << endl;
}

void BranchEndNode::load(ifstream& input_file) {
	string branch_start_node_id_line;
	getline(input_file, branch_start_node_id_line);
	this->branch_start_node_id = stoi(branch_start_node_id_line);

	string next_node_id_line;
	getline(input_file, next_node_id_line);
	this->next_node_id = stoi(next_node_id_line);
}

void BranchEndNode::link(Solution* parent_solution) {
	this->branch_start_node = this->parent->nodes[this->branch_start_node_id];

	if (this->next_node_id == -1) {
		this->next_node = NULL;
	} else {
		this->next_node = this->parent->nodes[this->next_node_id];
	}
}

void BranchEndNode::save_for_display(ofstream& output_file) {
	output_file << this->next_node_id << endl;
}
