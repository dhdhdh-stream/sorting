#include "exit_node.h"

#include <iostream>

#include "globals.h"
#include "scope.h"
#include "solution.h"

using namespace std;

ExitNode::ExitNode() {
	this->type = NODE_TYPE_EXIT;
}

ExitNode::ExitNode(ExitNode* original,
				   Solution* parent_solution) {
	this->type = NODE_TYPE_EXIT;

	this->exit_depth = original->exit_depth;

	this->next_node_parent = parent_solution->scopes[original->next_node_parent->id];
	this->next_node_id = original->next_node_id;
}

ExitNode::~ExitNode() {
	// do nothing
}

void ExitNode::save(ofstream& output_file) {
	output_file << this->exit_depth << endl;

	output_file << this->next_node_parent->id << endl;
	output_file << this->next_node_id << endl;
}

void ExitNode::load(ifstream& input_file,
					Solution* parent_solution) {
	string exit_depth_line;
	getline(input_file, exit_depth_line);
	this->exit_depth = stoi(exit_depth_line);

	string next_node_parent_id_line;
	getline(input_file, next_node_parent_id_line);
	this->next_node_parent = parent_solution->scopes[stoi(next_node_parent_id_line)];

	string next_node_id_line;
	getline(input_file, next_node_id_line);
	this->next_node_id = stoi(next_node_id_line);
}

void ExitNode::link(Solution* parent_solution) {
	this->next_node = this->next_node_parent->nodes[this->next_node_id];
}

void ExitNode::save_for_display(ofstream& output_file) {
	output_file << this->exit_depth << endl;

	output_file << this->next_node_parent->id << endl;
	output_file << this->next_node_id << endl;
}
