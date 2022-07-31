#include "solution_tree_node.h"

#include <iostream>

using namespace std;

SolutionTreeNode::SolutionTreeNode(SolutionTreeNode* parent,
								   int count,
								   double score,
								   double information) {
	this->parent = parent;
	this->count = count;
	this->score = score;
	this->information = information;
}

SolutionTreeNode::SolutionTreeNode(SolutionTreeNode* parent,
								   ifstream& save_file) {
	this->parent = parent;

	string has_halt_line;
	getline(save_file, has_halt_line);
	if (has_halt_line.compare("true")) {
		this->has_halt = true;
	} else {
		this->has_halt = false;
	}

	string count_line;
	getline(save_file, count_line);
	this->count = stoi(count_line);

	string score_line;
	getline(save_file, score_line);
	this->score = stof(score_line);

	string information_line;
	getline(save_file, information_line);
	this->information = stof(information_line);

	string num_children_line;
	getline(save_file, num_children_line);
	int num_children = stoi(num_children_line);
	for (int c_index = 0; c_index < num_children; c_index++) {
		string write_line;
		getline(save_file, write_line);
		double write_val = stof(write_line);
		
		string move_line;
		getline(save_file, move_line);
		int move_val = stoi(move_line);
		
		Action a(write_val, move_val);
		this->children_actions.push_back(a);

		string network_name_line;
		getline(save_file, network_name_line);
		this->children_network_names.push_back(network_name_line);

		SolutionTreeNode* child = new SolutionTreeNode(this, save_file);
		this->children.push_back(child);
	}
}

SolutionTreeNode::~SolutionTreeNode() {
	for (int c_index = 0; c_index < (int)this->children.size(); c_index++) {
		delete this->children[c_index];
		if (this->children_networks[c_index] != NULL) {
			delete this->children_networks[c_index];
		}
	}
}

void SolutionTreeNode::save(std::ofstream& save_file) {
	if (this->has_halt) {
		save_file << "true" << endl;
	} else {
		save_file << "false" << endl;
	}
	save_file << this->count << endl;
	save_file << this->score << endl;
	save_file << this->information << endl;
	save_file << this->children.size() << endl;
	for (int c_index = 0; c_index < (int)this->children.size(); c_index++) {
		save_file << this->children_actions[c_index].write << endl;
		save_file << this->children_actions[c_index].move << endl;
		save_file << this->children_network_names[c_index] << endl;
		this->children[c_index]->save(save_file);
	}
}
