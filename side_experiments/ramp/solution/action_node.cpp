#include "action_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "constants.h"
#include "scope.h"

using namespace std;

ActionNode::ActionNode() {
	this->type = NODE_TYPE_ACTION;

	this->experiment = NULL;
}

ActionNode::~ActionNode() {
	if (this->experiment != NULL) {
		delete this->experiment;
	}
}

void ActionNode::clean_inputs(Scope* scope,
							  int node_id) {
	if (this->experiment != NULL) {
		this->experiment->clean_inputs(scope,
									   node_id);
	}
}

void ActionNode::clean_inputs(Scope* scope) {
	if (this->experiment != NULL) {
		this->experiment->clean_inputs(scope);
	}
}

void ActionNode::replace_factor(Scope* scope,
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

void ActionNode::replace_obs_node(Scope* scope,
								  int original_node_id,
								  int new_node_id) {
	if (this->experiment != NULL) {
		this->experiment->replace_obs_node(scope,
										   original_node_id,
										   new_node_id);
	}
}

void ActionNode::replace_scope(Scope* original_scope,
							   Scope* new_scope,
							   int new_scope_node_id) {
	if (this->experiment != NULL) {
		this->experiment->replace_scope(original_scope,
										new_scope,
										new_scope_node_id);
	}
}

void ActionNode::save(ofstream& output_file) {
	this->action.save(output_file);

	output_file << this->next_node_id << endl;

	output_file << this->ancestor_ids.size() << endl;
	for (int a_index = 0; a_index < (int)this->ancestor_ids.size(); a_index++) {
		output_file << this->ancestor_ids[a_index] << endl;
	}
}

void ActionNode::load(ifstream& input_file) {
	this->action = Action(input_file);

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

void ActionNode::link(Solution* parent_solution) {
	if (this->next_node_id == -1) {
		this->next_node = NULL;
	} else {
		this->next_node = this->parent->nodes[this->next_node_id];
	}
}

void ActionNode::save_for_display(ofstream& output_file) {
	this->action.save(output_file);

	output_file << this->next_node_id << endl;
}

ActionNodeHistory::ActionNodeHistory(ActionNode* node) {
	this->node = node;
}
