#include "branch_node.h"

#include <iostream>

#include "abstract_experiment.h"
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
}

BranchNode::BranchNode(BranchNode* original) {
	this->type = NODE_TYPE_BRANCH;

	this->analyze_size = original->analyze_size;
	this->network = new Network(original->network);

	this->original_next_node_id = original->original_next_node_id;
	this->branch_next_node_id = original->branch_next_node_id;

	#if defined(MDEBUG) && MDEBUG
	this->verify_key = NULL;
	#endif /* MDEBUG */
}

BranchNode::~BranchNode() {
	delete this->network;

	for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
		this->experiments[e_index]->decrement(this);
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
	output_file << this->analyze_size << endl;
	this->network->save(output_file);

	output_file << this->original_next_node_id << endl;
	output_file << this->branch_next_node_id << endl;
}

void BranchNode::load(ifstream& input_file) {
	string analyze_size_line;
	getline(input_file, analyze_size_line);
	this->analyze_size = stoi(analyze_size_line);
	this->network = new Network(input_file);

	string original_next_node_id_line;
	getline(input_file, original_next_node_id_line);
	this->original_next_node_id = stoi(original_next_node_id_line);

	string branch_next_node_id_line;
	getline(input_file, branch_next_node_id_line);
	this->branch_next_node_id = stoi(branch_next_node_id_line);
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

void BranchNode::save_for_display(ofstream& output_file) {
	output_file << this->analyze_size << endl;

	output_file << this->original_next_node_id << endl;
	output_file << this->branch_next_node_id << endl;
}
