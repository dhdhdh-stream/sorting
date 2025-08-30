#include "start_node.h"

#include "abstract_experiment.h"
#include "scope.h"

using namespace std;

StartNode::StartNode() {
	this->type = NODE_TYPE_START;

	this->is_init = false;

	this->experiment = NULL;
}

StartNode::StartNode(StartNode* original) {
	this->type = NODE_TYPE_START;

	this->next_node_id = original->next_node_id;

	this->is_init = true;

	this->experiment = NULL;
}

void StartNode::clean() {
	// do nothing
}

void StartNode::measure_update(int total_count) {
	// do nothing
}

void StartNode::save(ofstream& output_file) {
	output_file << this->next_node_id << endl;
}

void StartNode::load(ifstream& input_file) {
	string next_node_id_line;
	getline(input_file, next_node_id_line);
	this->next_node_id = stoi(next_node_id_line);

	this->is_init = true;
}

void StartNode::link(Solution* parent_solution) {
	if (this->next_node_id == -1) {
		this->next_node = NULL;
	} else {
		this->next_node = this->parent->nodes[this->next_node_id];
	}
}

void StartNode::save_for_display(ofstream& output_file) {
	output_file << this->next_node_id << endl;
}

StartNodeHistory::StartNodeHistory(StartNode* node) {
	this->node = node;
}
