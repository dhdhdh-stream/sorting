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

	this->val_average = 0.0;
	this->start_damage = 0.0;
	this->end_damage = 0.0;

	this->sum_vals = 0.0;
	this->val_count = 0;
	this->sum_start_damage = 0.0;
	this->start_damage_count = 0;
	this->sum_end_damage = 0.0;
	this->end_damage_count = 0;
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

	output_file << this->val_average << endl;
	output_file << this->start_damage << endl;
	output_file << this->end_damage << endl;
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

	string val_average_line;
	getline(input_file, val_average_line);
	this->val_average = stod(val_average_line);

	string start_damage_line;
	getline(input_file, start_damage_line);
	this->start_damage = stod(start_damage_line);

	string end_damage_line;
	getline(input_file, end_damage_line);
	this->end_damage = stod(end_damage_line);
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

	this->is_damage = false;
	this->damage_new_scope = NULL;
}

ObsNodeHistory::~ObsNodeHistory() {
	if (this->damage_new_scope != NULL) {
		delete this->damage_new_scope;
	}

	for (int n_index = 0; n_index < (int)this->damage_nodes.size(); n_index++) {
		delete this->damage_nodes[n_index];
	}
}
