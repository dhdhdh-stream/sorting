#include "pass_through_node.h"

#include <iostream>

using namespace std;

PassThroughNode::PassThroughNode(int next_node_id) {
	this->type = NODE_TYPE_PASS_THROUGH;

	this->next_node_id = next_node_id;
}

PassThroughNode::PassThroughNode(ifstream& input_file,
								 int scope_id,
								 int scope_index) {
	this->type = NODE_TYPE_PASS_THROUGH;

	string next_node_id_line;
	getline(input_file, next_node_id_line);
	this->next_node_id = stoi(next_node_id_line);
}

void PassThroughNode::save(ofstream& output_file,
						   int scope_id,
						   int scope_index) {
	output_file << this->next_node_id << endl;
}

void PassThroughNode::save_for_display(ofstream& output_file) {
	output_file << this->next_node_id << endl;
}
