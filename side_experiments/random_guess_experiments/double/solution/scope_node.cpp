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
						 int& curr_1_index,
						 vector<int>& spots_0,
						 vector<bool>& switches_0,
						 vector<int>& spots_1,
						 vector<bool>& switches_1,
						 int& num_actions) {
	int curr_0_index_save = curr_0_index;
	bool curr_0_found = false;
	for (int i_index = 0; i_index < (int)this->indexes.size(); i_index++) {
		if (this->indexes[i_index] == curr_0_index) {
			curr_0_index = this->target_indexes[i_index];
			curr_0_found = true;
			break;
		}
	}
	if (!curr_0_found) {
		curr_0_index = -1;
	}

	int curr_1_index_save = curr_1_index;
	bool curr_1_found = false;
	for (int i_index = 0; i_index < (int)this->indexes.size(); i_index++) {
		if (this->indexes[i_index] == curr_1_index) {
			curr_1_index = this->target_indexes[i_index];
			curr_1_found = true;
			break;
		}
	}
	if (!curr_1_found) {
		curr_1_index = -1;
	}

	this->scope->activate(curr_spot,
						  curr_0_index,
						  curr_1_index,
						  spots_0,
						  switches_0,
						  spots_1,
						  switches_1,
						  num_actions);

	curr_0_index = curr_0_index_save;
	curr_1_index = curr_1_index_save;
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
					  int& curr_0_index,
					  int& curr_1_index) {
	int curr_0_index_save = curr_0_index;
	bool curr_0_found = false;
	for (int i_index = 0; i_index < (int)this->indexes.size(); i_index++) {
		if (this->indexes[i_index] == curr_0_index) {
			curr_0_index = this->target_indexes[i_index];
			curr_0_found = true;
			break;
		}
	}
	if (!curr_0_found) {
		curr_0_index = -1;
	}

	if (curr_0_index != curr_0_index_save) {
		cout << curr_0_index << " to " << curr_0_index_save << endl;
	}

	int curr_1_index_save = curr_1_index;
	bool curr_1_found = false;
	for (int i_index = 0; i_index < (int)this->indexes.size(); i_index++) {
		if (this->indexes[i_index] == curr_1_index) {
			curr_1_index = this->target_indexes[i_index];
			curr_1_found = true;
			break;
		}
	}
	if (!curr_1_found) {
		curr_1_index = -1;
	}

	if (curr_1_index != curr_1_index_save) {
		cout << curr_1_index << " to " << curr_1_index_save << endl;
	}

	this->scope->print(curr_spot,
					   curr_0_index,
					   curr_1_index);

	if (curr_0_index != curr_0_index_save) {
		cout << curr_0_index_save << " back to " << curr_0_index << endl;
	}
	curr_0_index = curr_0_index_save;

	if (curr_1_index != curr_1_index_save) {
		cout << curr_1_index_save << " back to " << curr_1_index << endl;
	}
	curr_1_index = curr_1_index_save;
}
