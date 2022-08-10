#include "compound_action.h"

#include <iostream>

using namespace std;

CompoundActionNode::CompoundActionNode(int next_child_index,
									   Action next_child_action) {
	this->children_indexes.push_back(next_child_index);
	this->children_actions.push_back(next_child_action);
}

CompoundActionNode::CompoundActionNode(ifstream& save_file) {
	string num_children_line;
	getline(save_file, num_children_line);
	int num_children = stoi(num_children_line);
	for (int c_index = 0; c_index < num_children; c_index++) {
		string child_index_line;
		getline(save_file, child_index_line);
		this->children_indexes.push_back(stoi(child_index_line));

		Action a(save_file);
		this->children_actions.push_back(a);
	}
}

CompoundActionNode::~CompoundActionNode() {
	// do nothing
}

void CompoundActionNode::save(ofstream& save_file) {
	save_file << this->children_indexes.size() << endl;
	for (int c_index = 0; c_index < (int)this->children_indexes.size(); c_index++) {
		save_file << this->children_indexes[c_index] << endl;
		this->children_actions[c_index].save(save_file);
	}
}

CompoundAction::CompoundAction(vector<Action> action_sequence) {
	CompoundActionNode* halt_node = new CompoundActionNode(-1, EMPTY_ACTION);
	this->nodes.push_back(halt_node);

	CompoundActionNode* root_node = new CompoundActionNode(2, action_sequence[0]);
	this->nodes.push_back(root_node);

	this->current_node_index = 2;

	for (int a_index = 1; a_index < (int)action_sequence.size()-1; a_index++) {
		CompoundActionNode* new_node = new CompoundActionNode(this->current_node_index+1, action_sequence[a_index]);
		this->nodes.push_back(new_node);
		this->current_node_index++;
	}

	CompoundActionNode* last_new_node = new CompoundActionNode(0, HALT);
	this->nodes.push_back(last_new_node);
	this->current_node_index++;
}

CompoundAction::CompoundAction(ifstream& save_file) {
	string num_nodes_line;
	getline(save_file, num_nodes_line);
	int num_nodes = stoi(num_nodes_line);
	for (int n_index = 0; n_index < num_nodes; n_index++) {
		CompoundActionNode* node = new CompoundActionNode(save_file);
		this->nodes.push_back(node);
	}
	this->current_node_index = num_nodes;
}

CompoundAction::~CompoundAction() {
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		delete this->nodes[n_index];
	}
}

void CompoundAction::save(ofstream& save_file) {
	save_file << this->nodes.size() << endl;
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		this->nodes[n_index]->save(save_file);
	}
}
