#include "exit_node.h"

#include <iostream>

#include "globals.h"
#include "scope.h"
#include "solution.h"

using namespace std;

ExitNode::ExitNode() {
	this->type = NODE_TYPE_EXIT;
}

ExitNode::~ExitNode() {
	// do nothing
}

void ExitNode::save(ofstream& output_file) {
	output_file << this->exit_depth << endl;

	output_file << this->next_node_parent_id << endl;
	output_file << this->next_node_id << endl;

	output_file << this->throw_id << endl;
}

void ExitNode::load(ifstream& input_file) {
	string exit_depth_line;
	getline(input_file, exit_depth_line);
	this->exit_depth = stoi(exit_depth_line);

	string next_node_parent_id_line;
	getline(input_file, next_node_parent_id_line);
	this->next_node_parent_id = stoi(next_node_parent_id_line);

	string next_node_id_line;
	getline(input_file, next_node_id_line);
	this->next_node_id = stoi(next_node_id_line);

	string throw_id_line;
	getline(input_file, throw_id_line);
	this->throw_id = stoi(throw_id_line);
}

void ExitNode::link() {
	if (this->throw_id == -1) {
		if (this->next_node_id == -1) {
			this->next_node = NULL;
		} else {
			this->next_node = solution->scopes[this->next_node_parent_id]->nodes[this->next_node_id];
		}
	}
}

void ExitNode::save_for_display(ofstream& output_file) {
	output_file << this->exit_depth << endl;

	output_file << this->next_node_parent_id << endl;
	output_file << this->next_node_id << endl;

	output_file << this->throw_id << endl;
}
