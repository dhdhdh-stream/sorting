#include "scope_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "globals.h"
#include "scope.h"
#include "solution.h"

using namespace std;

ScopeNode::ScopeNode() {
	this->type = NODE_TYPE_SCOPE;

	this->is_experiment = false;
	this->experiment_is_branch = false;
	this->experiment = NULL;

	this->average_instances_per_run = 0.0;
}

ScopeNode::ScopeNode(ScopeNode* original,
					 Solution* parent_solution) {
	this->type = NODE_TYPE_SCOPE;

	this->scope = parent_solution->scopes[original->scope->id];

	this->next_node_id = original->next_node_id;

	this->is_experiment = false;
	this->experiment_is_branch = false;
	this->experiment = NULL;

	this->average_instances_per_run = 0.0;
}

ScopeNode::~ScopeNode() {
	if (this->experiment != NULL) {
		delete this->experiment;
	}
}

void ScopeNode::save(ofstream& output_file) {
	output_file << this->scope->id << endl;

	output_file << this->next_node_id << endl;

	output_file << this->is_experiment << endl;

	output_file << this->average_instances_per_run << endl;
}

void ScopeNode::load(ifstream& input_file,
					 Solution* parent_solution) {
	string scope_id_line;
	getline(input_file, scope_id_line);
	this->scope = parent_solution->scopes[stoi(scope_id_line)];

	string next_node_id_line;
	getline(input_file, next_node_id_line);
	this->next_node_id = stoi(next_node_id_line);

	string is_experiment_line;
	getline(input_file, is_experiment_line);
	this->is_experiment = stoi(is_experiment_line);

	string average_instances_per_run_line;
	getline(input_file, average_instances_per_run_line);
	this->average_instances_per_run = stod(average_instances_per_run_line);
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