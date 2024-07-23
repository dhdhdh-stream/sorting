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

	this->average_instances_per_run = 0.0;

	#if defined(MDEBUG) && MDEBUG
	this->verify_key = NULL;
	#endif /* MDEBUG */
}

BranchNode::BranchNode(BranchNode* original) {
	this->type = NODE_TYPE_BRANCH;

	this->is_stub = original->is_stub;

	this->is_loop = original->is_loop;

	this->previous_location_id = original->previous_location_id;

	this->analyze_size = original->analyze_size;
	if (this->analyze_size == -1) {
		this->network = NULL;
	} else {
		this->network = new Network(original->network);
	}

	this->original_next_node_id = original->original_next_node_id;
	this->branch_next_node_id = original->branch_next_node_id;

	this->average_instances_per_run = 0.0;

	#if defined(MDEBUG) && MDEBUG
	this->verify_key = NULL;
	#endif /* MDEBUG */
}

BranchNode::~BranchNode() {
	if (this->network != NULL) {
		delete this->network;
	}

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
	output_file << this->is_stub << endl;

	output_file << this->is_loop << endl;

	output_file << this->previous_location_id << endl;

	output_file << this->analyze_size << endl;
	if (this->analyze_size != -1) {
		this->network->save(output_file);
	}

	output_file << this->original_next_node_id << endl;
	output_file << this->branch_next_node_id << endl;

	output_file << this->average_instances_per_run << endl;
}

void BranchNode::load(ifstream& input_file) {
	string is_stub_line;
	getline(input_file, is_stub_line);
	this->is_stub = stoi(is_stub_line);

	string is_loop_line;
	getline(input_file, is_loop_line);
	this->is_loop = stoi(is_loop_line);

	string previous_location_id_line;
	getline(input_file, previous_location_id_line);
	this->previous_location_id = stoi(previous_location_id_line);

	string analyze_size_line;
	getline(input_file, analyze_size_line);
	this->analyze_size = stoi(analyze_size_line);
	if (this->analyze_size != -1) {
		this->network = new Network(input_file);
	}

	string original_next_node_id_line;
	getline(input_file, original_next_node_id_line);
	this->original_next_node_id = stoi(original_next_node_id_line);

	string branch_next_node_id_line;
	getline(input_file, branch_next_node_id_line);
	this->branch_next_node_id = stoi(branch_next_node_id_line);

	string average_instances_per_run_line;
	getline(input_file, average_instances_per_run_line);
	this->average_instances_per_run = stod(average_instances_per_run_line);
}

void BranchNode::link(Solution* parent_solution) {
	if (this->previous_location_id == -1) {
		this->previous_location = NULL;
	} else {
		this->previous_location = this->parent->nodes[this->previous_location_id];
	}

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
	output_file << this->previous_location_id << endl;

	output_file << this->analyze_size << endl;

	output_file << this->original_next_node_id << endl;
	output_file << this->branch_next_node_id << endl;
}
