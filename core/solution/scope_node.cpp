#include "scope_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "globals.h"
#include "scope.h"
#include "solution.h"

using namespace std;

ScopeNode::ScopeNode() {
	this->type = NODE_TYPE_SCOPE;

	this->id = -1;
}

ScopeNode::ScopeNode(ScopeNode* original,
					 Solution* parent_solution) {
	this->type = NODE_TYPE_SCOPE;

	this->scope = parent_solution->scopes[original->scope->id];
	this->starting_node_id = original->starting_node_id;
	this->exit_node_ids = original->exit_node_ids;

	this->next_node_id = original->next_node_id;
}

ScopeNode::~ScopeNode() {
	for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
		delete this->experiments[e_index];
	}
}

void ScopeNode::save(ofstream& output_file) {
	output_file << this->scope->id << endl;
	output_file << this->starting_node_id << endl;
	output_file << this->exit_nodes.size() << endl;
	for (set<int>::iterator it = this->exit_node_ids.begin();
			it != this->exit_node_ids.end(); it++) {
		output_file << *it << endl;
	}

	output_file << this->next_node_id << endl;
}

void ScopeNode::load(ifstream& input_file,
					 Solution* parent_solution) {
	string scope_id_line;
	getline(input_file, scope_id_line);
	this->scope = parent_solution->scopes[stoi(scope_id_line)];

	string starting_node_id_line;
	getline(input_file, starting_node_id_line);
	this->starting_node_id = stoi(starting_node_id_line);

	string num_exit_node_ids_line;
	getline(input_file, num_exit_node_ids_line);
	int num_exit_node_ids = stoi(num_exit_node_ids_line);
	for (int e_index = 0; e_index < num_exit_node_ids; e_index++) {
		string node_id_line;
		getline(input_file, node_id_line);
		this->exit_node_ids.insert(stoi(node_id_line));
	}

	string next_node_id_line;
	getline(input_file, next_node_id_line);
	this->next_node_id = stoi(next_node_id_line);
}

void ScopeNode::link(Solution* parent_solution) {
	this->starting_node = this->scope->nodes[this->starting_node_id];
	for (set<int>::iterator it = this->exit_node_ids.begin();
			it != this->exit_node_ids.end(); it++) {
		this->exit_nodes.insert(this->scope->nodes[*it]);
	}

	if (this->next_node_id == -1) {
		this->next_node = NULL;
	} else {
		this->next_node = this->parent->nodes[this->next_node_id];
	}
}

void ScopeNode::save_for_display(ofstream& output_file) {
	output_file << this->scope->id << endl;
	output_file << this->starting_node_id << endl;

	output_file << this->next_node_id << endl;
}

ScopeNodeHistory::ScopeNodeHistory(ScopeNode* node) {
	this->node = node;
}

ScopeNodeHistory::ScopeNodeHistory(ScopeNodeHistory* original) {
	this->node = original->node;

	this->scope_history = new ScopeHistory(original->scope_history);

	this->normal_exit = original->normal_exit;
}

ScopeNodeHistory::~ScopeNodeHistory() {
	delete this->scope_history;
}
