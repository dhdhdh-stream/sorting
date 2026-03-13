#include "obs_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "constants.h"
#include "experiment.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"

using namespace std;

ObsNode::ObsNode() {
	this->type = NODE_TYPE_OBS;

	this->experiment = NULL;

	this->val_average = 0.0;
	this->average_hits_per_run = 0.0;

	this->sum_vals = 0.0;
	this->hit_count = 0;
}

ObsNode::~ObsNode() {
	if (this->experiment != NULL) {
		delete this->experiment;
	}
}

void ObsNode::update_val() {
	if (this->hit_count != 0) {
		this->val_average = this->sum_vals / this->hit_count;
		this->average_hits_per_run = (double)this->hit_count / (double)MEASURE_NUM_ITERS;

		this->sum_vals = 0.0;
		this->hit_count = 0;
	}
}

void ObsNode::save(ofstream& output_file) {
	output_file << this->next_node_id << endl;

	output_file << this->ancestor_ids.size() << endl;
	for (int a_index = 0; a_index < (int)this->ancestor_ids.size(); a_index++) {
		output_file << this->ancestor_ids[a_index] << endl;
	}

	output_file << this->val_average << endl;
	output_file << this->average_hits_per_run << endl;
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

	string average_hit_per_run_line;
	getline(input_file, average_hit_per_run_line);
	this->average_hits_per_run = stod(average_hit_per_run_line);
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
