#include "noop_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "globals.h"
#include "init_network.h"
#include "scope.h"
#include "score_network.h"
#include "solution.h"

using namespace std;

NoopNode::NoopNode() {
	this->type = NODE_TYPE_NOOP;

	this->average_instances_per_hit = 1.0;
	this->average_instances_per_run = 0.0;
	this->experiment = NULL;

	this->curr_num_instances = 0;
}

NoopNode::~NoopNode() {
	if (this->experiment != NULL) {
		delete this->experiment;
	}
}

void NoopNode::copy_from(NoopNode* original,
						 Solution* parent_solution) {
	this->next_node_id = original->next_node_id;

	this->average_instances_per_hit = original->average_instances_per_hit;
	this->average_instances_per_run = original->average_instances_per_run;

	this->ancestor_ids = original->ancestor_ids;
}

void NoopNode::save(ofstream& output_file) {
	output_file << this->next_node_id << endl;

	output_file << this->average_instances_per_hit << endl;
	output_file << this->average_instances_per_run << endl;

	output_file << this->ancestor_ids.size() << endl;
	for (int a_index = 0; a_index < (int)this->ancestor_ids.size(); a_index++) {
		output_file << this->ancestor_ids[a_index] << endl;
	}
}

void NoopNode::load(ifstream& input_file,
				   Solution* parent_solution) {
	string next_node_id_line;
	getline(input_file, next_node_id_line);
	this->next_node_id = stoi(next_node_id_line);

	string average_instances_per_hit_line;
	getline(input_file, average_instances_per_hit_line);
	this->average_instances_per_hit = stod(average_instances_per_hit_line);

	string average_instances_per_run_line;
	getline(input_file, average_instances_per_run_line);
	this->average_instances_per_run = stod(average_instances_per_run_line);

	string num_ancestors_line;
	getline(input_file, num_ancestors_line);
	int num_ancestors = stoi(num_ancestors_line);
	for (int a_index = 0; a_index < num_ancestors; a_index++) {
		string ancestor_id_line;
		getline(input_file, ancestor_id_line);
		this->ancestor_ids.push_back(stoi(ancestor_id_line));
	}
}

void NoopNode::link(Solution* parent_solution) {
	if (this->next_node_id == -1) {
		this->next_node = NULL;
	} else {
		this->next_node = this->parent->nodes[this->next_node_id];
	}
}

void NoopNode::save_for_display(ofstream& output_file) {
	output_file << this->next_node_id << endl;
}

NoopNodeHistory::NoopNodeHistory(NoopNode* node) {
	this->node = node;
}
