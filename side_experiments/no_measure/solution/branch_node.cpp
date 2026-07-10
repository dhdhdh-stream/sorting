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
}

BranchNode::~BranchNode() {
	for (int n_index = 0; n_index < (int)this->original_networks.size(); n_index++) {
		delete this->original_networks[n_index];
	}
	for (int n_index = 0; n_index < (int)this->branch_networks.size(); n_index++) {
		delete this->branch_networks[n_index];
	}

	if (this->original_experiment != NULL) {
		delete this->original_experiment;
	}
	if (this->branch_experiment != NULL) {
		delete this->branch_experiment;
	}
}

void BranchNode::save(ofstream& output_file) {
	output_file << this->original_networks.size() << endl;
	for (int l_index = 0; l_index < (int)this->original_networks.size(); l_index++) {
		this->original_networks[l_index]->save(output_file);
		this->branch_networks[l_index]->save(output_file);
		output_file << this->maintain_iters[l_index] << endl;
	}

	output_file << this->original_next_node_id << endl;
	output_file << this->branch_next_node_id << endl;

	output_file << this->ramp << endl;
	output_file << this->ramp_num_gears << endl;
	output_file << this->ramp_iter << endl;

	output_file << this->consec_original << endl;
	output_file << this->consec_branch << endl;

	output_file << this->ancestor_ids.size() << endl;
	for (int a_index = 0; a_index < (int)this->ancestor_ids.size(); a_index++) {
		output_file << this->ancestor_ids[a_index] << endl;
	}
}

void BranchNode::load(ifstream& input_file,
					  Solution* parent_solution) {
	string num_layers_line;
	getline(input_file, num_layers_line);
	int num_layers = stoi(num_layers_line);
	for (int l_index = 0; l_index < num_layers; l_index++) {
		this->original_networks.push_back(new Network(input_file));
		this->branch_networks.push_back(new Network(input_file));

		string iters_line;
		getline(input_file, iters_line);
		this->maintain_iters.push_back(stoi(iters_line));
	}

	string original_next_node_id_line;
	getline(input_file, original_next_node_id_line);
	this->original_next_node_id = stoi(original_next_node_id_line);

	string branch_next_node_id_line;
	getline(input_file, branch_next_node_id_line);
	this->branch_next_node_id = stoi(branch_next_node_id_line);

	string ramp_line;
	getline(input_file, ramp_line);
	this->ramp = stoi(ramp_line);

	string ramp_num_gears_line;
	getline(input_file, ramp_num_gears_line);
	this->ramp_num_gears = stoi(ramp_num_gears_line);

	// temp
	cout << "this->ramp_num_gears: " << this->ramp_num_gears << endl;

	string ramp_iter_line;
	getline(input_file, ramp_iter_line);
	this->ramp_iter = stoi(ramp_iter_line);

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

void BranchNode::save_for_display(ofstream& output_file) {
	output_file << this->original_next_node_id << endl;
	output_file << this->branch_next_node_id << endl;
}

BranchNodeHistory::BranchNodeHistory(BranchNode* node) {
	this->node = node;
}
