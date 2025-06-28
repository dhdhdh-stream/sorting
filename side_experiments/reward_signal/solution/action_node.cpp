#include "action_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "confusion.h"
#include "constants.h"
#include "scope.h"

using namespace std;

ActionNode::ActionNode() {
	this->type = NODE_TYPE_ACTION;

	this->is_init = false;

	this->experiment = NULL;
	this->confusion = NULL;

	this->last_updated_run_index = 0;
}

ActionNode::~ActionNode() {
	if (this->experiment != NULL) {
		this->experiment->decrement(this);
	}

	if (this->confusion != NULL) {
		delete this->confusion;
	}
}

void ActionNode::clean() {
	if (this->experiment != NULL) {
		this->experiment->decrement(this);
		this->experiment = NULL;
	}

	if (this->confusion != NULL) {
		delete this->confusion;
		this->confusion = NULL;
	}

	this->sum_score = 0.0;
	this->sum_hits = 0;
	this->sum_instances = 0;
}

void ActionNode::measure_update() {
	this->average_hits_per_run = (double)this->sum_hits / (double)MEASURE_ITERS;
	this->average_instances_per_run = (double)this->sum_instances / (double)this->sum_hits;
	this->average_score = this->sum_score / (double)this->sum_hits;
}

void ActionNode::new_scope_clean() {
	this->new_scope_sum_score = 0.0;
	this->new_scope_sum_count = 0;
}

void ActionNode::new_scope_measure_update(int total_count) {
	this->new_scope_average_hits_per_run = (double)this->new_scope_sum_count / (double)total_count;
	this->new_scope_average_score = this->new_scope_sum_score / (double)this->new_scope_sum_count;
}

void ActionNode::save(ofstream& output_file) {
	output_file << this->action << endl;

	output_file << this->next_node_id << endl;

	output_file << this->ancestor_ids.size() << endl;
	for (int a_index = 0; a_index < (int)this->ancestor_ids.size(); a_index++) {
		output_file << this->ancestor_ids[a_index] << endl;
	}

	output_file << this->average_hits_per_run << endl;
	output_file << this->average_score << endl;
}

void ActionNode::load(ifstream& input_file) {
	string action_line;
	getline(input_file, action_line);
	this->action = stoi(action_line);

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

	string average_hits_per_run_line;
	getline(input_file, average_hits_per_run_line);
	this->average_hits_per_run = stod(average_hits_per_run_line);

	string average_score_line;
	getline(input_file, average_score_line);
	this->average_score = stod(average_score_line);

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
}
