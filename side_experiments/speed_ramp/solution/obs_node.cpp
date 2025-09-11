#include "obs_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "constants.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"

using namespace std;

ObsNode::ObsNode() {
	this->type = NODE_TYPE_OBS;

	this->experiment = NULL;
}

ObsNode::ObsNode(ObsNode* original) {
	this->type = NODE_TYPE_OBS;

	this->next_node_id = original->next_node_id;

	this->ancestor_ids = original->ancestor_ids;

	this->experiment = NULL;
}

ObsNode::~ObsNode() {
	if (this->experiment != NULL) {
		delete this->experiment;
	}
}

void ObsNode::save(ofstream& output_file) {
	output_file << this->next_node_id << endl;

	output_file << this->ancestor_ids.size() << endl;
	for (int a_index = 0; a_index < (int)this->ancestor_ids.size(); a_index++) {
		output_file << this->ancestor_ids[a_index] << endl;
	}
}

void ObsNode::load(ifstream& input_file,
				   Solution* parent_solution) {
	string next_node_id_line;
	getline(input_file, next_node_id_line);
	this->next_node_id = stoi(next_node_id_line);

	string num_ancestors_line;
	getline(input_file, num_ancestors_line);
	int num_ancestors = stoi(num_ancestors_line);
	for (int a_index = 0; a_index < num_ancestors; a_index++) {
		string ancestor_id_line;
		getline(input_file, ancestor_id_line);
		this->ancestor_ids.push_back(stoi(ancestor_id_line));
	}
}

void ObsNode::link(Solution* parent_solution) {
	if (this->next_node_id == -1) {
		this->next_node = NULL;
	} else {
		this->next_node = this->parent->nodes[this->next_node_id];
	}
}

void ObsNode::save_for_display(ofstream& output_file) {
	output_file << this->next_node_id << endl;
}

ObsNodeHistory::ObsNodeHistory(ObsNode* node) {
	this->node = node;
}

ObsNodeHistory::ObsNodeHistory(ObsNodeHistory* original) {
	this->node = original->node;
	this->index = original->index;

	this->obs_history = original->obs_history;
}
