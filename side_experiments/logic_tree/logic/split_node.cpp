#include "split_node.h"

#include "logic_tree.h"
#include "network.h"

using namespace std;

SplitNode::SplitNode() {
	this->type = LOGIC_NODE_TYPE_SPLIT;

	this->experiment = NULL;
}

SplitNode::~SplitNode() {
	delete this->network;
}

void SplitNode::save(ofstream& output_file) {
	this->network->save(output_file);

	output_file << this->original_node_id << endl;
	output_file << this->branch_node_id << endl;

	output_file << this->weight << endl;
}

void SplitNode::load(ifstream& input_file) {
	this->network = new Network(input_file);

	string original_node_id_line;
	getline(input_file, original_node_id_line);
	this->original_node_id = stoi(original_node_id_line);

	string branch_node_id_line;
	getline(input_file, branch_node_id_line);
	this->branch_node_id = stoi(branch_node_id_line);

	string weight_line;
	getline(input_file, weight_line);
	this->weight = stod(weight_line);
}

void SplitNode::link(LogicTree* logic_tree) {
	this->original_node = logic_tree->nodes[this->original_node_id];
	this->branch_node = logic_tree->nodes[this->branch_node_id];
}
