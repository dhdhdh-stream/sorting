#include "info_branch_node.h"

#include "abstract_experiment.h"
#include "globals.h"
#include "info_scope.h"
#include "scope.h"
#include "solution.h"
#include "solution_set.h"

using namespace std;

InfoBranchNode::InfoBranchNode() {
	this->type = NODE_TYPE_INFO_BRANCH;
}

InfoBranchNode::InfoBranchNode(InfoBranchNode* original,
							   Solution* parent_solution) {
	this->type = NODE_TYPE_INFO_BRANCH;

	this->scope = parent_solution->info_scopes[original->scope->id];
	this->is_negate = original->is_negate;

	this->branch_end_node_id = original->branch_end_node_id;

	this->original_next_node_id = original->original_next_node_id;
	this->branch_next_node_id = original->branch_next_node_id;
}

InfoBranchNode::~InfoBranchNode() {
	for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
		this->experiments[e_index]->decrement(this);
	}
}

void InfoBranchNode::save(ofstream& output_file) {
	output_file << this->scope->id << endl;
	output_file << this->is_negate << endl;

	output_file << this->branch_end_node_id << endl;

	output_file << this->original_next_node_id << endl;
	output_file << this->branch_next_node_id << endl;
}

void InfoBranchNode::load(ifstream& input_file,
						  Solution* parent_solution) {
	string scope_id_line;
	getline(input_file, scope_id_line);
	this->scope = parent_solution->info_scopes[stoi(scope_id_line)];

	string is_negate_line;
	getline(input_file, is_negate_line);
	this->is_negate = stoi(is_negate_line);

	string branch_end_node_id_line;
	getline(input_file, branch_end_node_id_line);
	this->branch_end_node_id = stoi(branch_end_node_id_line);

	string original_next_node_id_line;
	getline(input_file, original_next_node_id_line);
	this->original_next_node_id = stoi(original_next_node_id_line);

	string branch_next_node_id_line;
	getline(input_file, branch_next_node_id_line);
	this->branch_next_node_id = stoi(branch_next_node_id_line);
}

void InfoBranchNode::link(Solution* parent_solution) {
	this->branch_end_node = (BranchEndNode*)this->parent->nodes[this->branch_end_node_id];

	if (this->original_next_node_id == -1) {
		this->original_next_node = NULL;
	} else {
		this->original_next_node = this->parent->nodes[this->original_next_node_id];
	}

	if (this->branch_next_node_id == -1) {
		this->branch_next_node = NULL;
	} else {
		this->branch_next_node = this->parent->nodes[this->branch_next_node_id];
	}
}

void InfoBranchNode::save_for_display(ofstream& output_file) {
	output_file << this->scope->id << endl;

	output_file << this->original_next_node_id << endl;
	output_file << this->branch_next_node_id << endl;
}

InfoBranchNodeHistory::InfoBranchNodeHistory() {
	// do nothing
}

InfoBranchNodeHistory::InfoBranchNodeHistory(InfoBranchNodeHistory* original) {
	this->index = original->index;

	this->is_branch = original->is_branch;
}
