#include "action_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "scope.h"

using namespace std;

ActionNode::ActionNode() {
	this->type = NODE_TYPE_ACTION;

	this->average_instances_per_run = 0.0;
}

ActionNode::ActionNode(ActionNode* original) {
	this->type = NODE_TYPE_ACTION;

	this->action = original->action;

	this->next_node_id = original->next_node_id;

	this->average_instances_per_run = 0.0;
}

ActionNode::~ActionNode() {
	for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
		this->experiments[e_index]->decrement(this);
	}
}

void ActionNode::clean_inputs(int scope_id,
							  int node_id) {
	for (int i_index = (int)this->input_scope_context_ids.size()-1; i_index >= 0; i_index--) {
		bool is_match = false;
		for (int l_index = 0; l_index < (int)this->input_scope_context_ids[i_index].size(); l_index++) {
			if (this->input_scope_context_ids[i_index][l_index] == scope_id
					&& this->input_node_context_ids[i_index][l_index] == node_id) {
				is_match = true;
				break;
			}
		}

		if (is_match) {
			this->input_scope_context_ids.erase(this->input_scope_context_ids.begin() + i_index);
			this->input_node_context_ids.erase(this->input_node_context_ids.begin() + i_index);
			this->input_obs_indexes.erase(this->input_obs_indexes.begin() + i_index);
		}
	}
}

void ActionNode::update_scope_ids(map<int, int>& dictionary) {
	for (int i_index = 0; i_index < (int)this->input_scope_context_ids.size(); i_index++) {
		for (int l_index = 0; l_index < (int)this->input_scope_context_ids[i_index].size(); l_index++) {
			this->input_scope_context_ids[i_index][l_index] =
				dictionary[this->input_scope_context_ids[i_index][l_index]];
		}
	}
}

void ActionNode::save(ofstream& output_file) {
	this->action.save(output_file);

	output_file << this->next_node_id << endl;

	output_file << this->average_instances_per_run << endl;
}

void ActionNode::load(ifstream& input_file) {
	this->action = Action(input_file);

	string next_node_id_line;
	getline(input_file, next_node_id_line);
	this->next_node_id = stoi(next_node_id_line);

	string average_instances_per_run_line;
	getline(input_file, average_instances_per_run_line);
	this->average_instances_per_run = stod(average_instances_per_run_line);
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

ActionNodeHistory::ActionNodeHistory(ActionNodeHistory* original) {
	this->node = original->node;
	this->index = original->index;

	this->obs_history = original->obs_history;
}
