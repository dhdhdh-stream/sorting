#include "branch_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "scope.h"
#include "solution.h"

using namespace std;

BranchNode::BranchNode() {
	this->type = NODE_TYPE_BRANCH;

	this->experiment = NULL;
}

BranchNode::~BranchNode() {
	delete this->original_network;
	delete this->branch_network;

	if (this->experiment != NULL) {
		delete this->experiment;
	}
}

void BranchNode::save(ofstream& output_file) {
	this->original_network->save(output_file);
	this->branch_network->save(output_file);

	output_file << this->original_next_node_id << endl;
	output_file << this->branch_next_node_id << endl;

	output_file << this->consec_original << endl;
	output_file << this->consec_branch << endl;

	output_file << this->ancestor_ids.size() << endl;
	for (int a_index = 0; a_index < (int)this->ancestor_ids.size(); a_index++) {
		output_file << this->ancestor_ids[a_index] << endl;
	}
}

void BranchNode::load(ifstream& input_file,
					  Solution* parent_solution) {
	this->original_network = new Network(input_file);
	this->branch_network = new Network(input_file);

	string original_next_node_id_line;
	getline(input_file, original_next_node_id_line);
	this->original_next_node_id = stoi(original_next_node_id_line);

	string branch_next_node_id_line;
	getline(input_file, branch_next_node_id_line);
	this->branch_next_node_id = stoi(branch_next_node_id_line);

	string consec_original_line;
	getline(input_file, consec_original_line);
	this->consec_original = stoi(consec_original_line);

	// temp
	if (this->consec_original >= CONSEC_DEPRECATE_LIMIT) {
		cout << "this->consec_original >= CONSEC_DEPRECATE_LIMIT" << endl;
	}

	string consec_branch_line;
	getline(input_file, consec_branch_line);
	this->consec_branch = stoi(consec_branch_line);

	// temp
	if (this->consec_branch >= CONSEC_DEPRECATE_LIMIT) {
		cout << "this->consec_branch >= CONSEC_DEPRECATE_LIMIT" << endl;
	}

	string num_ancestors_line;
	getline(input_file, num_ancestors_line);
	int num_ancestors = stoi(num_ancestors_line);
	for (int a_index = 0; a_index < num_ancestors; a_index++) {
		string ancestor_id_line;
		getline(input_file, ancestor_id_line);
		this->ancestor_ids.push_back(stoi(ancestor_id_line));
	}
}

void BranchNode::link(Solution* parent_solution) {
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

void BranchNode::copy_from(BranchNode* original,
						   Solution* parent_solution) {
	this->original_network = new Network(original->original_network);
	this->branch_network = new Network(original->branch_network);

	this->original_next_node_id = original->original_next_node_id;
	this->branch_next_node_id = original->branch_next_node_id;

	this->consec_original = original->consec_original;
	this->consec_branch = original->consec_branch;

	this->ancestor_ids = original->ancestor_ids;
}

void BranchNode::save_for_display(ofstream& output_file) {
	output_file << this->original_next_node_id << endl;
	output_file << this->branch_next_node_id << endl;
}

BranchNodeHistory::BranchNodeHistory(BranchNode* node) {
	this->node = node;
}
