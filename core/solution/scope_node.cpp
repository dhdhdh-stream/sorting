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

ScopeNode::~ScopeNode() {
	for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
		delete this->experiments[e_index];
	}
}

void ScopeNode::reset() {
	for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
		delete this->experiments[e_index];
	}
	this->experiments.clear();
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

	output_file << this->catches.size() << endl;
	for (map<int, int>::iterator it = this->catch_ids.begin();
			it != this->catch_ids.end(); it++) {
		output_file << it->first << endl;
		output_file << it->second << endl;
	}
}

void ScopeNode::load(ifstream& input_file) {
	string scope_id_line;
	getline(input_file, scope_id_line);
	this->scope = solution->scopes[stoi(scope_id_line)];

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

	string num_catches_line;
	getline(input_file, num_catches_line);
	int num_catches = stoi(num_catches_line);
	for (int c_index = 0; c_index < num_catches; c_index++) {
		string throw_id_line;
		getline(input_file, throw_id_line);

		string node_id_line;
		getline(input_file, node_id_line);

		this->catch_ids[stoi(throw_id_line)] = stoi(node_id_line);
	}
}

void ScopeNode::link() {
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

	for (map<int, int>::iterator it = this->catch_ids.begin();
			it != this->catch_ids.end(); it++) {
		if (it->second == -1) {
			this->catches[it->first] = NULL;
		} else {
			this->catches[it->first] = this->parent->nodes[it->second];
		}
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

	this->throw_id = original->throw_id;
}

ScopeNodeHistory::~ScopeNodeHistory() {
	delete this->scope_history;
}
