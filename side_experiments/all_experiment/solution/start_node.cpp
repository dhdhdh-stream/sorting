#include "start_node.h"

#include "abstract_experiment.h"
#include "scope.h"

using namespace std;

StartNode::StartNode() {
	this->type = NODE_TYPE_START;

	this->val_average = 0.0;
	this->start_damage = 0.0;
	this->end_damage = 0.0;

	this->sum_vals = 0.0;
	this->val_count = 0;
	this->sum_start_damage = 0.0;
	this->start_damage_count = 0;
	this->sum_end_damage = 0.0;
	this->end_damage_count = 0;
}

void StartNode::save(ofstream& output_file) {
	output_file << this->next_node_id << endl;

	output_file << this->val_average << endl;
	output_file << this->start_damage << endl;
	output_file << this->end_damage << endl;
}

void StartNode::load(ifstream& input_file) {
	string next_node_id_line;
	getline(input_file, next_node_id_line);
	this->next_node_id = stoi(next_node_id_line);

	string val_average_line;
	getline(input_file, val_average_line);
	this->val_average = stod(val_average_line);

	string start_damage_line;
	getline(input_file, start_damage_line);
	this->start_damage = stod(start_damage_line);

	string end_damage_line;
	getline(input_file, end_damage_line);
	this->end_damage = stod(end_damage_line);
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
