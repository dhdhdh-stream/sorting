#include "start_node.h"

#include "abstract_experiment.h"
#include "scope.h"

using namespace std;

StartNode::StartNode() {
	this->type = NODE_TYPE_START;

	this->experiment = NULL;
}

StartNode::~StartNode() {
	if (this->experiment != NULL) {
		delete this->experiment;
	}
}

void StartNode::save(ofstream& output_file) {
	output_file << this->next_node_id << endl;
}

void StartNode::load(ifstream& input_file) {
	string next_node_id_line;
	getline(input_file, next_node_id_line);
	this->next_node_id = stoi(next_node_id_line);
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

StartNodeHistory::StartNodeHistory(StartNodeHistory* original) {
	this->node = original->node;
	this->index = original->index;
	this->num_actions_snapshot = original->num_actions_snapshot;
}
