#include "solution_node.h"

#include <iostream>
#include <boost/algorithm/string/trim.hpp>

using namespace std;

SolutionNode::SolutionNode(int node_index) {
	this->node_index = node_index;
	this->count = 0;
	this->information = 0.0;
}

SolutionNode::SolutionNode(int node_index,
						   ifstream& save_file) {
	this->node_index = node_index;

	string count_line;
	getline(save_file, count_line);
	this->count = stoi(count_line);

	string information_line;
	getline(save_file, information_line);
	this->information = stof(information_line);

	string num_children_line;
	getline(save_file, num_children_line);
	int num_children = stoi(num_children_line);
	for (int c_index = 0; c_index < num_children; c_index++) {
		string child_index_line;
		getline(save_file, child_index_line);
		this->children_indexes.push_back(stoi(child_index_line));

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
		boost::algorithm::trim(network_name_line);
		this->children_network_names.push_back(network_name_line);

		if (network_name_line.length() > 10) {
			ifstream save_file;
			save_file.open(network_name_line);
			Network* child_network = new Network(save_file);
			save_file.close();
			this->children_networks.push_back(child_network);
		} else {
			this->children_networks.push_back(NULL);
		}
	}
}

SolutionNode::~SolutionNode() {
	for (int c_index = 0; c_index < (int)this->children_networks.size(); c_index++) {
		if (this->children_networks[c_index] != NULL) {
			delete this->children_networks[c_index];
		}
	}
}

void SolutionNode::save(std::ofstream& save_file) {
	save_file << this->count << endl;
	save_file << this->information << endl;
	save_file << this->children_indexes.size() << endl;
	for (int c_index = 0; c_index < (int)this->children_indexes.size(); c_index++) {
		save_file << this->children_indexes[c_index] << endl;
		save_file << this->children_actions[c_index].write << endl;
		save_file << this->children_actions[c_index].move << endl;
		save_file << this->children_network_names[c_index] << endl;
	}
}
