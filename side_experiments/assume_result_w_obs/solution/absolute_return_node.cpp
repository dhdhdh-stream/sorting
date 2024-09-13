#include "absolute_return_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"

using namespace std;

AbsoluteReturnNode::AbsoluteReturnNode() {
	this->type = NODE_TYPE_ABSOLUTE_RETURN;

	this->average_instances_per_run = 0.0;
}

AbsoluteReturnNode::AbsoluteReturnNode(AbsoluteReturnNode* original) {
	this->type = NODE_TYPE_ABSOLUTE_RETURN;

	this->location = original->location;

	this->next_node_id = original->next_node_id;

	this->average_instances_per_run = 0.0;
}

AbsoluteReturnNode::~AbsoluteReturnNode() {
	for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
		this->experiments[e_index]->decrement(this);
	}
}

void AbsoluteReturnNode::save(ofstream& output_file) {
	output_file << this->location.size() << endl;
	for (int l_index = 0; l_index < (int)this->location.size(); l_index++) {
		output_file << this->location[l_index] << endl;
	}

	output_file << this->next_node_id << endl;

	output_file << this->average_instances_per_run << endl;
}

void AbsoluteReturnNode::load(ifstream& input_file) {
	string location_size_line;
	getline(input_file, location_size_line);
	int location_size = stoi(location_size_line);
	for (int l_index = 0; l_index < location_size; l_index++) {
		string component_line;
		getline(input_file, component_line);
		this->location.push_back(stoi(component_line));
	}

	string next_node_id_line;
	getline(input_file, next_node_id_line);
	this->next_node_id = stoi(next_node_id_line);

	string average_instances_per_run_line;
	getline(input_file, average_instances_per_run_line);
	this->average_instances_per_run = stod(average_instances_per_run_line);
}

void AbsoluteReturnNode::link(Solution* parent_solution) {
	if (this->next_node_id == -1) {
		this->next_node = NULL;
	} else {
		this->next_node = this->parent->nodes[this->next_node_id];
	}
}

void AbsoluteReturnNode::save_for_display(ofstream& output_file) {
	output_file << this->next_node_id << endl;
}
