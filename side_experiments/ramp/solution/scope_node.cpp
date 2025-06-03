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

	this->experiment = NULL;
}

ScopeNode::ScopeNode(ScopeNode* original,
					 Solution* parent_solution) {
	this->type = NODE_TYPE_SCOPE;

	this->scope = parent_solution->scopes[original->scope->id];

	this->next_node_id = original->next_node_id;

	this->ancestor_ids = original->ancestor_ids;

	this->experiment = NULL;
}

ScopeNode::~ScopeNode() {
	if (this->experiment != NULL) {
		this->experiment->decrement(this);
	}
}

void ScopeNode::clean_inputs(Scope* scope,
							 int node_id) {
	if (this->experiment != NULL) {
		this->experiment->clean_inputs(scope,
									   node_id);
	}
}

void ScopeNode::clean_inputs(Scope* scope) {
	if (this->experiment != NULL) {
		this->experiment->clean_inputs(scope);
	}
}

void ScopeNode::replace_factor(Scope* scope,
							   int original_node_id,
							   int original_factor_index,
							   int new_node_id,
							   int new_factor_index) {
	if (this->experiment != NULL) {
		this->experiment->replace_factor(scope,
										 original_node_id,
										 original_factor_index,
										 new_node_id,
										 new_factor_index);
	}
}

void ScopeNode::replace_obs_node(Scope* scope,
								 int original_node_id,
								 int new_node_id) {
	if (this->experiment != NULL) {
		this->experiment->replace_obs_node(scope,
										   original_node_id,
										   new_node_id);
	}
}

void ScopeNode::replace_scope(Scope* original_scope,
							  Scope* new_scope,
							  int new_scope_node_id) {
	if (this->scope == original_scope) {
		this->scope = new_scope;
	}

	if (this->experiment != NULL) {
		this->experiment->replace_scope(original_scope,
										new_scope,
										new_scope_node_id);
	}
}

void ScopeNode::clean() {
	if (this->experiment != NULL) {
		this->experiment->decrement(this);
		this->experiment = NULL;
	}
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

ScopeNodeHistory::~ScopeNodeHistory() {
	delete this->scope_history;
}
