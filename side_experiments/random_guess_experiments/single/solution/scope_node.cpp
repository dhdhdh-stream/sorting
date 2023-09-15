#include "scope_node.h"

#include <iostream>

using namespace std;

ScopeNode::ScopeNode(vector<int> indexes,
					 vector<int> target_indexes,
					 Scope* scope) {
	this->type = NODE_TYPE_SCOPE;

	this->indexes = indexes;
	this->target_indexes = target_indexes;
	this->scope = scope;
}

ScopeNode::ScopeNode(ScopeNode* original) {
	this->type = NODE_TYPE_SCOPE;

	this->indexes = original->indexes;
	this->target_indexes = original->target_indexes;
	this->scope = new Scope(original->scope);
}

ScopeNode::~ScopeNode() {
	delete this->scope;
}

void ScopeNode::activate(int& curr_spot,
						 int& curr_0_index,
						 vector<int>& spots,
						 vector<bool>& switches,
						 int& num_actions) {
	int curr_0_index_save = curr_0_index;
	bool found = false;
	for (int i_index = 0; i_index < (int)this->indexes.size(); i_index++) {
		if (this->indexes[i_index] == curr_0_index) {
			curr_0_index = this->target_indexes[i_index];
			found = true;
			break;
		}
	}
	if (!found) {
		curr_0_index = -1;
	}

	this->scope->activate(curr_spot,
						  curr_0_index,
						  spots,
						  switches,
						  num_actions);

	curr_0_index = curr_0_index_save;
}

void ScopeNode::fetch_context(vector<Scope*>& scope_context,
							  vector<int>& node_context,
							  int& curr_num_action,
							  int target_num_action) {
	this->scope->fetch_context(scope_context,
							   node_context,
							   curr_num_action,
							   target_num_action);
}

void ScopeNode::print(int& curr_spot,
					  int& curr_0_index) {
	int curr_0_index_save = curr_0_index;
	bool found = false;
	for (int i_index = 0; i_index < (int)this->indexes.size(); i_index++) {
		if (this->indexes[i_index] == curr_0_index) {
			curr_0_index = this->target_indexes[i_index];
			found = true;
			break;
		}
	}
	if (!found) {
		curr_0_index = -1;
	}

	if (curr_0_index != curr_0_index_save) {
		cout << curr_0_index_save << " to " << curr_0_index << endl;
	}

	this->scope->print(curr_spot,
					   curr_0_index);

	if (curr_0_index != curr_0_index_save) {
		cout << curr_0_index << " back to " << curr_0_index_save << endl;
	}

	curr_0_index = curr_0_index_save;
}
