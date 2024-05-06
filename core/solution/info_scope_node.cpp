#include "info_scope_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "globals.h"
#include "info_scope.h"
#include "scope.h"
#include "solution.h"

using namespace std;

InfoScopeNode::InfoScopeNode() {
	this->type = NODE_TYPE_INFO_SCOPE;
}

InfoScopeNode::InfoScopeNode(InfoScopeNode* original,
							 Solution* parent_solution) {
	this->type = NODE_TYPE_INFO_SCOPE;

	this->scope = parent_solution->info_scopes[original->scope->id];

	this->next_node_id = original->next_node_id;
}

InfoScopeNode::~InfoScopeNode() {
	for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
		delete this->experiments[e_index];
	}
}

void InfoScopeNode::save(ofstream& output_file) {
	output_file << this->scope->id << endl;

	output_file << this->next_node_id << endl;
}

void InfoScopeNode::load(ifstream& input_file) {
	string scope_id_line;
	getline(input_file, scope_id_line);
	this->scope = solution->info_scopes[stoi(scope_id_line)];

	string next_node_id_line;
	getline(input_file, next_node_id_line);
	this->next_node_id = stoi(next_node_id_line);
}

void InfoScopeNode::link(Solution* parent_solution) {
	if (this->next_node_id == -1) {
		this->next_node = NULL;
	} else {
		this->next_node = this->parent->nodes[this->next_node_id];
	}
}

/**
 * - unused
 */
void InfoScopeNode::link() {
	// do nothing
}

void InfoScopeNode::save_for_display(ofstream& output_file) {
	output_file << this->scope->id << endl;

	output_file << this->next_node_id << endl;
}

InfoScopeNodeHistory::InfoScopeNodeHistory() {
	this->scope_history = NULL;
}

InfoScopeNodeHistory::InfoScopeNodeHistory(InfoScopeNodeHistory* original) {
	this->index = original->index;

	/**
	 * - don't copy scope_history
	 */
	this->scope_history = NULL;

	this->is_positive = original->is_positive;
}

InfoScopeNodeHistory::~InfoScopeNodeHistory() {
	if (this->scope_history != NULL) {
		delete this->scope_history;
	}
}
