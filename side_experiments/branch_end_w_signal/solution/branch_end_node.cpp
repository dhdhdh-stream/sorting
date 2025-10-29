#include "branch_end_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "network.h"
#include "scope.h"

using namespace std;

BranchEndNode::BranchEndNode() {
	this->type = NODE_TYPE_BRANCH_END;

	this->signal_network = NULL;

	this->experiment = NULL;
}

BranchEndNode::~BranchEndNode() {
	if (this->signal_network != NULL) {
		delete this->signal_network;
	}

	if (this->experiment != NULL) {
		delete this->experiment;
	}
}

void BranchEndNode::save(ofstream& output_file) {
	output_file << (this->signal_network == NULL) << endl;
	if (this->signal_network != NULL) {
		this->signal_network->save(output_file);
	}

	output_file << this->next_node_id << endl;

	output_file << this->branch_start_node_id << endl;

	output_file << this->ancestor_ids.size() << endl;
	for (int a_index = 0; a_index < (int)this->ancestor_ids.size(); a_index++) {
		output_file << this->ancestor_ids[a_index] << endl;
	}
}

void BranchEndNode::load(ifstream& input_file,
						 Solution* parent_solution) {
	string signal_network_is_null_line;
	getline(input_file, signal_network_is_null_line);
	bool signal_network_is_null = stoi(signal_network_is_null_line);
	if (!signal_network_is_null) {
		this->signal_network = new Network(input_file);
	}

	string next_node_id_line;
	getline(input_file, next_node_id_line);
	this->next_node_id = stoi(next_node_id_line);

	string branch_start_node_id_line;
	getline(input_file, branch_start_node_id_line);
	this->branch_start_node_id = stoi(branch_start_node_id_line);

	string num_ancestors_line;
	getline(input_file, num_ancestors_line);
	int num_ancestors = stoi(num_ancestors_line);
	for (int a_index = 0; a_index < num_ancestors; a_index++) {
		string ancestor_id_line;
		getline(input_file, ancestor_id_line);
		this->ancestor_ids.push_back(stoi(ancestor_id_line));
	}
}

void BranchEndNode::link(Solution* parent_solution) {
	if (this->next_node_id == -1) {
		this->next_node = NULL;
	} else {
		this->next_node = this->parent->nodes[this->next_node_id];
	}

	this->branch_start_node = (BranchNode*)this->parent->nodes[this->branch_start_node_id];
}

void BranchEndNode::save_for_display(ofstream& output_file) {
	output_file << this->next_node_id << endl;
}

BranchEndNodeHistory::BranchEndNodeHistory(BranchEndNode* node) {
	this->node = node;

	this->signal_sum_vals = 0.0;
	this->signal_sum_counts = 0;
}
