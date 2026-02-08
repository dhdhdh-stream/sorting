#include "decision_tree.h"

#include <iostream>
#include <set>

#include "decision_tree_node.h"

using namespace std;

DecisionTree::DecisionTree() {
	this->node_counter = 0;

	this->root = NULL;

	this->history_index = 0;
}

DecisionTree::~DecisionTree() {
	for (map<int, DecisionTreeNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		delete it->second;
	}
}

void DecisionTree::save(ofstream& output_file) {
	output_file << this->node_counter << endl;

	output_file << this->nodes.size() << endl;
	for (map<int, DecisionTreeNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		output_file << it->first << endl;
		it->second->save(output_file);
	}

	if (this->root == NULL) {
		output_file << -1 << endl;
	} else {
		output_file << this->root->id << endl;
	}

	output_file << this->improvement_history.size() << endl;
	for (int h_index = 0; h_index < (int)this->improvement_history.size(); h_index++) {
		output_file << this->improvement_history[h_index] << endl;
	}
}

void DecisionTree::load(ifstream& input_file) {
	string node_counter_line;
	getline(input_file, node_counter_line);
	this->node_counter = stoi(node_counter_line);

	string num_nodes_line;
	getline(input_file, num_nodes_line);
	int num_nodes = stoi(num_nodes_line);
	for (int n_index = 0; n_index < num_nodes; n_index++) {
		string id_line;
		getline(input_file, id_line);
		int id = stoi(id_line);

		DecisionTreeNode* decision_tree_node = new DecisionTreeNode();
		decision_tree_node->id = id;
		decision_tree_node->load(input_file);
		this->nodes[decision_tree_node->id] = decision_tree_node;
	}

	for (map<int, DecisionTreeNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		it->second->link(this);
	}

	string root_id_line;
	getline(input_file, root_id_line);
	int root_id = stoi(root_id_line);
	if (root_id == -1) {
		this->root = NULL;
	} else {
		this->root = this->nodes[root_id];
	}

	string history_size_line;
	getline(input_file, history_size_line);
	int history_size = stoi(history_size_line);
	for (int h_index = 0; h_index < history_size; h_index++) {
		string improvement_line;
		getline(input_file, improvement_line);
		this->improvement_history.push_back(stod(improvement_line));
	}
}

void DecisionTree::copy_from(DecisionTree* original) {
	this->node_counter = original->node_counter;

	for (map<int, DecisionTreeNode*>::iterator it = original->nodes.begin();
			it != original->nodes.end(); it++) {
		DecisionTreeNode* decision_tree_node = new DecisionTreeNode();
		decision_tree_node->id = it->first;
		decision_tree_node->copy_from(it->second);
		this->nodes[it->first] = decision_tree_node;
	}

	if (original->root == NULL) {
		this->root = NULL;
	} else {
		this->root = this->nodes[original->root->id];
	}

	this->improvement_history = original->improvement_history;

	this->obs_histories = original->obs_histories;
	this->target_val_histories = original->target_val_histories;
	this->history_index = original->history_index;
}
