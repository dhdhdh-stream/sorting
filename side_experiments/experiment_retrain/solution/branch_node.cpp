#include "branch_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "constants.h"
#include "globals.h"
#include "init_network.h"
#include "scope.h"
#include "score_network.h"
#include "solution.h"

using namespace std;

BranchNode::BranchNode() {
	this->type = NODE_TYPE_BRANCH;

	this->original_average_instances_per_hit = 1.0;
	this->original_average_instances_per_run = 0.0;
	this->original_experiment = NULL;
	this->branch_average_instances_per_hit = 1.0;
	this->branch_average_instances_per_run = 0.0;
	this->branch_experiment = NULL;

	this->original_curr_num_instances = 0;
	this->branch_curr_num_instances = 0;
}

BranchNode::~BranchNode() {
	for (int n_index = 0; n_index < (int)this->init_networks.size(); n_index++) {
		delete this->init_networks[n_index];
	}

	delete this->original_network;
	delete this->branch_network;

	if (this->original_experiment != NULL) {
		delete this->original_experiment;
	}
	if (this->branch_experiment != NULL) {
		delete this->branch_experiment;
	}
}

void BranchNode::save(ofstream& output_file) {
	output_file << this->init_networks.size() << endl;
	for (int n_index = 0; n_index < (int)this->init_networks.size(); n_index++) {
		this->init_networks[n_index]->save(output_file);
	}

	this->original_network->save(output_file);
	this->branch_network->save(output_file);

	output_file << this->original_next_node_id << endl;
	output_file << this->branch_next_node_id << endl;

	output_file << this->ramp << endl;
	output_file << this->ramp_num_gears << endl;
	output_file << this->ramp_iter << endl;

	output_file << this->consec_original << endl;
	output_file << this->consec_branch << endl;

	output_file << this->original_average_instances_per_hit << endl;
	output_file << this->original_average_instances_per_run << endl;
	output_file << this->branch_average_instances_per_hit << endl;
	output_file << this->branch_average_instances_per_run << endl;

	output_file << this->ancestor_ids.size() << endl;
	for (int a_index = 0; a_index < (int)this->ancestor_ids.size(); a_index++) {
		output_file << this->ancestor_ids[a_index] << endl;
	}
}

void BranchNode::load(ifstream& input_file,
					  Solution* parent_solution) {
	string num_init_networks_line;
	getline(input_file, num_init_networks_line);
	int num_init_networks = stoi(num_init_networks_line);
	for (int n_index = 0; n_index < num_init_networks; n_index++) {
		this->init_networks.push_back(new InitNetwork(input_file));
	}

	this->original_network = new ScoreNetwork(input_file);
	this->branch_network = new ScoreNetwork(input_file);

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

	string original_average_instances_per_hit_line;
	getline(input_file, original_average_instances_per_hit_line);
	this->original_average_instances_per_hit = stod(original_average_instances_per_hit_line);

	string original_average_instances_per_run_line;
	getline(input_file, original_average_instances_per_run_line);
	this->original_average_instances_per_run = stod(original_average_instances_per_run_line);

	string branch_average_instances_per_hit_line;
	getline(input_file, branch_average_instances_per_hit_line);
	this->branch_average_instances_per_hit = stod(branch_average_instances_per_hit_line);

	string branch_average_instances_per_run_line;
	getline(input_file, branch_average_instances_per_run_line);
	this->branch_average_instances_per_run = stod(branch_average_instances_per_run_line);

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
