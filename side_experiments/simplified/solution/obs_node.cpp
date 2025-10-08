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

	this->num_experiments = 0;

	this->last_updated_run_index = -1;

	this->experiment = NULL;
}

ObsNode::~ObsNode() {
	if (this->experiment != NULL) {
		delete this->experiment;
	}
}

void ObsNode::clean() {
	if (this->experiment != NULL) {
		delete this->experiment;
		this->experiment = NULL;
	}

	this->last_updated_run_index = -1;
	this->sum_score = 0.0;
	this->sum_hits = 0;
	this->sum_instances = 0;
}

void ObsNode::measure_update(int total_count) {
	this->average_hits_per_run = (double)this->sum_hits / (double)total_count;
	this->average_instances_per_run = (double)this->sum_instances / (double)this->sum_hits;
	this->average_score = this->sum_score / (double)this->sum_hits;
}

void ObsNode::save(ofstream& output_file) {
	output_file << this->next_node_id << endl;

	output_file << this->ancestor_ids.size() << endl;
	for (int a_index = 0; a_index < (int)this->ancestor_ids.size(); a_index++) {
		output_file << this->ancestor_ids[a_index] << endl;
	}

	output_file << this->num_experiments << endl;
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

	string num_experiments_line;
	getline(input_file, num_experiments_line);
	this->num_experiments = stoi(num_experiments_line);
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
	this->num_actions_snapshot = original->num_actions_snapshot;

	this->obs_history = original->obs_history;
}
