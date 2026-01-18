#include "branch_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "scope.h"
#include "solution.h"

using namespace std;

const double ATTRIBUTE_UPDATE_RATE = 0.02;

BranchNode::BranchNode() {
	this->type = NODE_TYPE_BRANCH;

	this->network = NULL;

	this->original_impact = 0.0;
	this->branch_impact = 0.0;

	this->original_update_sum_vals = 0.0;
	this->original_update_counts = 0;
	this->branch_update_sum_vals = 0.0;
	this->branch_update_counts = 0;

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

void BranchNode::attribute_update() {
	if (this->original_update_counts > 0) {
		this->original_impact += ATTRIBUTE_UPDATE_RATE * this->original_update_sum_vals / this->original_update_counts;

		this->original_update_sum_vals = 0.0;
		this->original_update_counts = 0;
	}
	if (this->branch_update_counts > 0) {
		this->branch_impact += ATTRIBUTE_UPDATE_RATE * this->branch_update_sum_vals / this->branch_update_counts;

		this->branch_update_sum_vals = 0.0;
		this->branch_update_counts = 0;
	}
}

void BranchNode::save(ofstream& output_file) {
	this->network->save(output_file);

	output_file << this->original_next_node_id << endl;
	output_file << this->branch_next_node_id << endl;

	output_file << this->original_impact << endl;
	output_file << this->branch_impact << endl;

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

	string original_impact_line;
	getline(input_file, original_impact_line);
	this->original_impact = stod(original_impact_line);

	string branch_impact_line;
	getline(input_file, branch_impact_line);
	this->branch_impact = stod(branch_impact_line);

	// temp
	cout << "this->parent->id: " << this->parent->id << endl;
	cout << "this->id: " << this->id << endl;
	cout << "this->original_impact: " << this->original_impact << endl;
	cout << "this->branch_impact: " << this->branch_impact << endl;

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

	this->original_impact = original->original_impact;
	this->branch_impact = original->branch_impact;

	this->ancestor_ids = original->ancestor_ids;
}

void BranchNode::save_for_display(ofstream& output_file) {
	output_file << this->original_next_node_id << endl;
	output_file << this->branch_next_node_id << endl;
}

BranchNodeHistory::BranchNodeHistory(BranchNode* node) {
	this->node = node;
}
