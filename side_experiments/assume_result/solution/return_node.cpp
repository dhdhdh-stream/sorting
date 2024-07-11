#include "return_node.h"

#include "abstract_experiment.h"
#include "scope.h"

using namespace std;

ReturnNode::ReturnNode() {
	this->type = NODE_TYPE_RETURN;
}

ReturnNode::ReturnNode(ReturnNode* original) {
	this->type = NODE_TYPE_RETURN;

	this->previous_location_id = original->previous_location_id;

	this->next_node_id = original->next_node_id;
}

ReturnNode::~ReturnNode() {
	for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
		this->experiments[e_index]->decrement(this);
	}
}

void ReturnNode::save(ofstream& output_file) {
	output_file << this->previous_location_id << endl;

	output_file << this->next_node_id << endl;
}

void ReturnNode::load(ifstream& input_file) {
	string previous_location_id_line;
	getline(input_file, previous_location_id_line);
	this->previous_location_id = stoi(previous_location_id_line);

	string next_node_id_line;
	getline(input_file, next_node_id_line);
	this->next_node_id = stoi(next_node_id_line);
}

void ReturnNode::link(Solution* parent_solution) {
	if (this->previous_location_id == -1) {
		this->previous_location = NULL;
	} else {
		this->previous_location = this->parent->nodes[this->previous_location_id];
	}

	if (this->next_node_id == -1) {
		this->next_node = NULL;
	} else {
		this->next_node = this->parent->nodes[this->next_node_id];
	}
}

void ReturnNode::save_for_display(ofstream& output_file) {
	output_file << this->previous_location_id << endl;

	output_file << this->next_node_id << endl;
}
