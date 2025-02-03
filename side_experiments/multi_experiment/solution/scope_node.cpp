#include "scope_node.h"

#include "abstract_experiment.h"
#include "globals.h"
#include "scope.h"
#include "solution.h"

using namespace std;

ScopeNode::ScopeNode() {
	this->type = NODE_TYPE_SCOPE;
}

ScopeNode::~ScopeNode() {
	for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
		this->experiments[e_index]->decrement(this);
	}
}

void ScopeNode::clean_inputs(Scope* scope,
							 int node_id) {
	for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
		this->experiments[e_index]->clean_inputs(scope,
												 node_id);
	}
}

void ScopeNode::clean_inputs(Scope* scope) {
	for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
		this->experiments[e_index]->clean_inputs(scope);
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

void ScopeNode::load(ifstream& input_file) {
	string scope_id_line;
	getline(input_file, scope_id_line);
	this->scope = solution->scopes[stoi(scope_id_line)];

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

void ScopeNode::link() {
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
