#include "exit_node.h"

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
	output_file << this->is_exit << endl;
	output_file << this->exit_depth << endl;

	output_file << this->exit_node_parent_id << endl;
	output_file << this->exit_node_id << endl;
}

void ExitNode::load(ifstream& input_file) {
	string is_exit_line;
	getline(input_file, is_exit_line);
	this->is_exit = stoi(is_exit_line);

	string exit_depth_line;
	getline(input_file, exit_depth_line);
	this->exit_depth = stoi(exit_depth_line);

	string exit_node_parent_id_line;
	getline(input_file, exit_node_parent_id_line);
	this->exit_node_parent_id = stoi(exit_node_parent_id_line);

	string exit_node_id_line;
	getline(input_file, exit_node_id_line);
	this->exit_node_id = stoi(exit_node_id_line);
}

void ExitNode::link() {
	if (this->exit_node_id == -1) {
		this->exit_node = NULL;
	} else {
		this->exit_node = solution->scopes[this->exit_node_parent_id]->nodes[this->exit_node_id];
	}
}

void ExitNode::save_for_display(ofstream& output_file) {
	output_file << this->is_exit << endl;
	output_file << this->exit_depth << endl;

	output_file << this->exit_node_parent_id << endl;
	output_file << this->exit_node_id << endl;
}
