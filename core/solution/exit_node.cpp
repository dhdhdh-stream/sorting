#include "exit_node.h"

using namespace std;

ExitNode::ExitNode() {
	this->type = NODE_TYPE_EXIT;

	this->id = -1;
}

ExitNode::ExitNode(ifstream& input_file,
				   int id) {
	this->type = NODE_TYPE_EXIT;

	this->id = id;

	string exit_depth_line;
	getline(input_file, exit_depth_line);
	this->exit_depth = stoi(exit_depth_line);

	string exit_node_id_line;
	getline(input_file, exit_node_id_line);
	this->exit_node_id = stoi(exit_node_id_line);
}

ExitNode::~ExitNode() {
	// do nothing
}

void ExitNode::save(ofstream& output_file) {
	output_file << this->exit_depth << endl;
	output_file << this->exit_node_id << endl;
}

void ExitNode::save_for_display(ofstream& output_file) {
	output_file << this->exit_depth << endl;
	output_file << this->exit_node_id << endl;
}
