#include "start_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "solution.h"
#include "world_model.h"
#include "wrapper.h"

using namespace std;

StartNode::StartNode() {
	this->type = NODE_TYPE_START;

	this->history_index = 0;
	this->experiment = NULL;

	this->curr_instances_per_run = 0;
}

StartNode::~StartNode() {
	if (this->experiment != NULL) {
		delete this->experiment;
	}
}

void StartNode::save(ofstream& output_file,
					 Wrapper* wrapper) {
	output_file << this->next_node_id << endl;

	output_file << this->average_instances_per_run << endl;
}

void StartNode::load(ifstream& input_file,
					 Wrapper* wrapper) {
	string next_node_id_line;
	getline(input_file, next_node_id_line);
	this->next_node_id = stoi(next_node_id_line);

	string average_instances_per_run_line;
	getline(input_file, average_instances_per_run_line);
	this->average_instances_per_run = stod(average_instances_per_run_line);
}

void StartNode::link(Wrapper* wrapper) {
	if (this->next_node_id == -1) {
		this->next_node = NULL;
	} else {
		this->next_node = wrapper->solution->nodes[this->next_node_id];
	}
}

void StartNode::save_for_display(ofstream& output_file) {
	output_file << this->next_node_id << endl;
}

StartNodeHistory::StartNodeHistory(StartNode* node) {
	this->node = node;
}
