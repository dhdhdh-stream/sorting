#include "scope_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "constants.h"
#include "globals.h"
#include "scope.h"
#include "solution.h"

using namespace std;

ScopeNode::ScopeNode() {
	this->type = NODE_TYPE_SCOPE;

	this->is_init = false;

	this->experiment = NULL;

	this->last_updated_run_index = -1;
}

ScopeNode::~ScopeNode() {
	if (this->experiment != NULL) {
		this->experiment->decrement(this);
	}
}

void ScopeNode::clean() {
	if (this->experiment != NULL) {
		this->experiment->decrement(this);
		this->experiment = NULL;
	}

	this->last_updated_run_index = -1;
	this->sum_hits = 0;
	this->sum_instances = 0;
}

void ScopeNode::measure_update(int total_count) {
	this->average_hits_per_run = (double)this->sum_hits / (double)total_count;
	this->average_instances_per_run = (double)this->sum_instances / (double)this->sum_hits;
}

void ScopeNode::save(ofstream& output_file) {
	output_file << this->scope->id << endl;

	output_file << this->next_node_id << endl;

	output_file << this->ancestor_ids.size() << endl;
	for (int a_index = 0; a_index < (int)this->ancestor_ids.size(); a_index++) {
		output_file << this->ancestor_ids[a_index] << endl;
	}
}

void ScopeNode::load(ifstream& input_file,
					 Solution* parent_solution) {
	string scope_id_line;
	getline(input_file, scope_id_line);
	this->scope = parent_solution->scopes[stoi(scope_id_line)];

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

	this->is_init = true;
}

void ScopeNode::link(Solution* parent_solution) {
	if (this->next_node_id == -1) {
		this->next_node = NULL;
	} else {
		this->next_node = this->parent->nodes[this->next_node_id];
	}
}

void ScopeNode::save_for_display(ofstream& output_file) {
	output_file << this->scope->id << endl;

	output_file << this->next_node_id << endl;
}

ScopeNodeHistory::ScopeNodeHistory(ScopeNode* node) {
	this->node = node;
}

ScopeNodeHistory::ScopeNodeHistory(ScopeNodeHistory* original) {
	this->node = original->node;
	this->index = original->index;

	this->scope_history = new ScopeHistory(original->scope_history);
}

ScopeNodeHistory::~ScopeNodeHistory() {
	delete this->scope_history;
}
