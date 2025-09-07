#include "action_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "constants.h"
#include "scope.h"

using namespace std;

ActionNode::ActionNode() {
	this->type = NODE_TYPE_ACTION;

	this->is_init = false;

	this->num_experiments = 0;

	this->experiment = NULL;

	this->last_updated_run_index = -1;
}

ActionNode::ActionNode(ActionNode* original) {
	this->type = NODE_TYPE_ACTION;

	this->action = original->action;

	this->next_node_id = original->next_node_id;

	this->average_hits_per_run = original->average_hits_per_run;
	this->average_instances_per_run = original->average_instances_per_run;

	this->ancestor_ids = original->ancestor_ids;

	this->is_init = true;

	this->num_experiments = original->num_experiments;

	this->experiment = NULL;

	this->last_updated_run_index = -1;
}

void ActionNode::clean() {
	this->last_updated_run_index = -1;
	this->sum_hits = 0;
	this->sum_instances = 0;
}

void ActionNode::measure_update(int total_count) {
	this->average_hits_per_run = (double)this->sum_hits / (double)total_count;
	this->average_instances_per_run = (double)this->sum_instances / (double)this->sum_hits;
}

void ActionNode::save(ofstream& output_file) {
	output_file << this->action << endl;

	output_file << this->next_node_id << endl;

	output_file << this->average_hits_per_run << endl;
	output_file << this->average_instances_per_run << endl;

	output_file << this->ancestor_ids.size() << endl;
	for (int a_index = 0; a_index < (int)this->ancestor_ids.size(); a_index++) {
		output_file << this->ancestor_ids[a_index] << endl;
	}

	output_file << this->num_experiments << endl;
}

void ActionNode::load(ifstream& input_file) {
	string action_line;
	getline(input_file, action_line);
	this->action = stoi(action_line);

	string next_node_id_line;
	getline(input_file, next_node_id_line);
	this->next_node_id = stoi(next_node_id_line);

	string average_hits_per_run_line;
	getline(input_file, average_hits_per_run_line);
	this->average_hits_per_run = stod(average_hits_per_run_line);

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

	string num_experiments_line;
	getline(input_file, num_experiments_line);
	this->num_experiments = stoi(num_experiments_line);

	this->is_init = true;
}

void ActionNode::link(Solution* parent_solution) {
	if (this->next_node_id == -1) {
		this->next_node = NULL;
	} else {
		this->next_node = this->parent->nodes[this->next_node_id];
	}
}

void ActionNode::save_for_display(ofstream& output_file) {
	output_file << this->action << endl;
	output_file << this->next_node_id << endl;
}

ActionNodeHistory::ActionNodeHistory(ActionNode* node) {
	this->node = node;
}

ActionNodeHistory::ActionNodeHistory(ActionNodeHistory* original) {
	this->node = original->node;
	this->index = original->index;
}
