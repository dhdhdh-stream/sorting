#include "return_node.h"

#include "abstract_experiment.h"
#include "scope.h"

using namespace std;

ReturnNode::ReturnNode() {
	this->type = NODE_TYPE_RETURN;

	this->average_instances_per_run = 0.0;
}

ReturnNode::ReturnNode(ReturnNode* original) {
	this->type = NODE_TYPE_RETURN;

	this->previous_location_id = original->previous_location_id;

	this->passed_next_node_id = original->passed_next_node_id;
	this->skipped_next_node_id = original->skipped_next_node_id;

	this->average_instances_per_run = 0.0;
}

ReturnNode::~ReturnNode() {
	for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
		this->experiments[e_index]->decrement(this);
	}
}

void ReturnNode::save(ofstream& output_file) {
	output_file << this->previous_location_id << endl;

	output_file << this->passed_next_node_id << endl;
	output_file << this->skipped_next_node_id << endl;

	output_file << this->average_instances_per_run << endl;
}

void ReturnNode::load(ifstream& input_file) {
	string previous_location_id_line;
	getline(input_file, previous_location_id_line);
	this->previous_location_id = stoi(previous_location_id_line);

	string passed_next_node_id_line;
	getline(input_file, passed_next_node_id_line);
	this->passed_next_node_id = stoi(passed_next_node_id_line);

	string skipped_next_node_id_line;
	getline(input_file, skipped_next_node_id_line);
	this->skipped_next_node_id = stoi(skipped_next_node_id_line);

	string average_instances_per_run_line;
	getline(input_file, average_instances_per_run_line);
	this->average_instances_per_run = stod(average_instances_per_run_line);
}

void ReturnNode::link(Solution* parent_solution) {
	if (this->previous_location_id == -1) {
		this->previous_location = NULL;
	} else {
		this->previous_location = this->parent->nodes[this->previous_location_id];
	}

	if (this->passed_next_node_id == -1) {
		this->passed_next_node = NULL;
	} else {
		this->passed_next_node = this->parent->nodes[this->passed_next_node_id];
	}
	if (this->skipped_next_node_id == -1) {
		this->skipped_next_node = NULL;
	} else {
		this->skipped_next_node = this->parent->nodes[this->skipped_next_node_id];
	}
}

void ReturnNode::save_for_display(ofstream& output_file) {
	output_file << this->previous_location_id << endl;

	output_file << this->passed_next_node_id << endl;
	output_file << this->skipped_next_node_id << endl;
}
