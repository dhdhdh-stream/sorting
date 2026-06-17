#include "branch_node.h"

#include "abstract_experiment.h"
#include "network.h"
#include "solution.h"
#include "world_model.h"
#include "wrapper.h"

using namespace std;

BranchNode::BranchNode() {
	this->type = NODE_TYPE_BRANCH;

	this->original_history_index = 0;
	this->original_experiment = NULL;
	this->branch_history_index = 0;
	this->branch_experiment = NULL;

	this->original_curr_instances_per_run = 0;
	this->branch_curr_instances_per_run = 0;
}

BranchNode::~BranchNode() {
	delete this->original_network;
	delete this->branch_network;

	if (this->original_experiment != NULL) {
		delete this->original_experiment;
	}
	if (this->branch_experiment != NULL) {
		delete this->branch_experiment;
	}
}

void BranchNode::save(ofstream& output_file,
					  Wrapper* wrapper) {
	this->original_network->save(output_file);
	this->branch_network->save(output_file);

	output_file << this->original_next_node_id << endl;
	output_file << this->branch_next_node_id << endl;

	output_file << this->ramp << endl;
	output_file << this->ramp_iter << endl;

	output_file << this->original_average_instances_per_run << endl;
	output_file << this->branch_average_instances_per_run << endl;

	output_file << this->ancestor_ids.size() << endl;
	for (int a_index = 0; a_index < (int)this->ancestor_ids.size(); a_index++) {
		output_file << this->ancestor_ids[a_index] << endl;
	}
}

void BranchNode::load(ifstream& input_file,
					  Wrapper* wrapper) {
	this->original_network = new Network(input_file);
	this->branch_network = new Network(input_file);

	string original_next_node_id_line;
	getline(input_file, original_next_node_id_line);
	this->original_next_node_id = stoi(original_next_node_id_line);

	string branch_next_node_id_line;
	getline(input_file, branch_next_node_id_line);
	this->branch_next_node_id = stoi(branch_next_node_id_line);

	string ramp_line;
	getline(input_file, ramp_line);
	this->ramp = stoi(ramp_line);

	string ramp_iter_line;
	getline(input_file, ramp_iter_line);
	this->ramp_iter = stoi(ramp_iter_line);

	string original_average_instances_per_run_line;
	getline(input_file, original_average_instances_per_run_line);
	this->original_average_instances_per_run = stod(original_average_instances_per_run_line);

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

void BranchNode::link(Wrapper* wrapper) {
	if (this->original_next_node_id == -1) {
		this->original_next_node = NULL;
	} else {
		this->original_next_node = wrapper->solution->nodes[this->original_next_node_id];
	}

	if (this->branch_next_node_id == -1) {
		this->branch_next_node = NULL;
	} else {
		this->branch_next_node = wrapper->solution->nodes[this->branch_next_node_id];
	}
}

void BranchNode::save_for_display(ofstream& output_file) {
	output_file << this->original_next_node_id << endl;
	output_file << this->branch_next_node_id << endl;
}

BranchNodeHistory::BranchNodeHistory(BranchNode* node) {
	this->node = node;
}
