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

	this->input_scope_context_ids = original->input_scope_context_ids;
	this->input_node_context_ids = original->input_node_context_ids;
	this->input_obs_indexes = original->input_obs_indexes;

	this->next_node_id = original->next_node_id;

	this->average_instances_per_run = 0.0;
}

ActionNode::~ActionNode() {
	for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
		this->experiments[e_index]->decrement(this);
	}
}

void ActionNode::save(ofstream& output_file) {
	this->action.save(output_file);

	output_file << this->input_scope_context_ids.size() << endl;
	for (int i_index = 0; i_index < (int)this->input_scope_context_ids.size(); i_index++) {
		output_file << this->input_scope_context_ids[i_index].size() << endl;
		for (int l_index = 0; l_index < (int)this->input_scope_context_ids[i_index].size(); l_index++) {
			output_file << this->input_scope_context_ids[i_index][l_index] << endl;
			output_file << this->input_node_context_ids[i_index][l_index] << endl;
		}

		output_file << this->input_obs_indexes[i_index] << endl;
	}

	output_file << this->next_node_id << endl;

	output_file << this->average_instances_per_run << endl;
}

void ActionNode::load(ifstream& input_file) {
	this->action = Action(input_file);

	string num_inputs_line;
	getline(input_file, num_inputs_line);
	int num_inputs = stoi(num_inputs_line);
	for (int i_index = 0; i_index < num_inputs; i_index++) {
		this->input_scope_context_ids.push_back(vector<int>());
		this->input_node_context_ids.push_back(vector<int>());

		string num_layers_line;
		getline(input_file, num_layers_line);
		int num_layers = stoi(num_layers_line);
		for (int l_index = 0; l_index < num_layers; l_index++) {
			string scope_id_line;
			getline(input_file, scope_id_line);
			this->input_scope_context_ids[i_index].push_back(stoi(scope_id_line));

			string node_id_line;
			getline(input_file, node_id_line);
			this->input_node_context_ids[i_index].push_back(stoi(node_id_line));
		}

		string obs_index_line;
		getline(input_file, obs_index_line);
		this->input_obs_indexes.push_back(stoi(obs_index_line));
	}

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

ActionNodeHistory::ActionNodeHistory() {
	// do nothing
}

ActionNodeHistory::ActionNodeHistory(ActionNodeHistory* original) {
	this->obs_history = original->obs_history;
}
