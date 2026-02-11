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

	this->network = NULL;

	#if defined(MDEBUG) && MDEBUG
	this->verify_key = NULL;
	#endif /* MDEBUG */

	this->experiment = NULL;
}

BranchNode::~BranchNode() {
	if (this->network != NULL) {
		delete this->network;
	}

	if (this->experiment != NULL) {
		delete this->experiment;
	}
}

#if defined(MDEBUG) && MDEBUG
void BranchNode::clear_verify() {
	this->verify_key = NULL;
	if (this->verify_scores.size() > 0) {
		cout << "seed: " << seed << endl;

		throw invalid_argument("branch node remaining verify");
	}
}
#endif /* MDEBUG */

void BranchNode::save(ofstream& output_file) {
	this->network->save(output_file);

	output_file << this->original_next_node_id << endl;
	output_file << this->branch_next_node_id << endl;

	output_file << this->ancestor_ids.size() << endl;
	for (int a_index = 0; a_index < (int)this->ancestor_ids.size(); a_index++) {
		output_file << this->ancestor_ids[a_index] << endl;
	}
}

void BranchNode::load(ifstream& input_file,
					  Solution* parent_solution) {
	this->network = new Network(input_file);

	string original_next_node_id_line;
	getline(input_file, original_next_node_id_line);
	this->original_next_node_id = stoi(original_next_node_id_line);

	string branch_next_node_id_line;
	getline(input_file, branch_next_node_id_line);
	this->branch_next_node_id = stoi(branch_next_node_id_line);

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
	this->network = new Network(original->network);

	this->original_next_node_id = original->original_next_node_id;
	this->branch_next_node_id = original->branch_next_node_id;

	this->ancestor_ids = original->ancestor_ids;
}

void BranchNode::save_for_display(ofstream& output_file) {
	output_file << this->original_next_node_id << endl;
	output_file << this->branch_next_node_id << endl;
}

BranchNodeHistory::BranchNodeHistory(BranchNode* node) {
	this->node = node;
}
