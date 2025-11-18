#include "split_node.h"

#include "logic_tree.h"
#include "network.h"

using namespace std;

SplitNode::SplitNode() {
	this->type = LOGIC_NODE_TYPE_SPLIT;

	this->experiment = NULL;
}

SplitNode::~SplitNode() {
	// do nothing
}

void SplitNode::save(ofstream& output_file) {
	output_file << this->obs_index << endl;
	output_file << this->rel_obs_index << endl;
	output_file << this->split_type << endl;
	output_file << this->split_target << endl;
	output_file << this->split_range << endl;

	output_file << this->original_node_id << endl;
	output_file << this->branch_node_id << endl;

	output_file << this->weight << endl;
}

void SplitNode::load(ifstream& input_file) {
	string obs_index_line;
	getline(input_file, obs_index_line);
	this->obs_index = stoi(obs_index_line);

	string rel_obs_index_line;
	getline(input_file, rel_obs_index_line);
	this->rel_obs_index = stoi(rel_obs_index_line);

	string split_type_line;
	getline(input_file, split_type_line);
	this->split_type = stoi(split_type_line);

	string split_target_line;
	getline(input_file, split_target_line);
	this->split_target = stod(split_target_line);

	string split_range_line;
	getline(input_file, split_range_line);
	this->split_range = stod(split_range_line);

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
