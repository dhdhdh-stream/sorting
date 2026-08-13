#include "action_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "action_network.h"
#include "constants.h"
#include "init_network.h"
#include "obs_network.h"
#include "scope.h"
#include "score_network.h"
#include "solution.h"

using namespace std;

ActionNode::ActionNode() {
	this->type = NODE_TYPE_ACTION;

	this->average_instances_per_hit = 1.0;
	this->average_instances_per_run = 0.0;
	this->experiment = NULL;

	this->curr_num_instances = 0;
}

ActionNode::~ActionNode() {
	delete this->action_network;

	delete this->obs_network;

	for (int n_index = 0; n_index < (int)this->init_networks.size(); n_index++) {
		delete this->init_networks[n_index];
	}

	delete this->score_network;

	if (this->experiment != NULL) {
		delete this->experiment;
	}
}

void ActionNode::copy_from(ActionNode* original,
						   Solution* parent_solution) {
	this->action = original->action;

	this->action_network = new ActionNetwork(original->action_network);

	this->obs_network = new ObsNetwork(original->obs_network);

	for (int n_index = 0; n_index < (int)original->init_network_scope_contexts.size(); n_index++) {
		vector<Scope*> scope_context;
		for (int l_index = 0; l_index < (int)original->init_network_scope_contexts[n_index].size(); l_index++) {
			scope_context.push_back(parent_solution->scopes[original->init_network_scope_contexts[n_index][l_index]->id]);
		}
		this->init_network_scope_contexts.push_back(scope_context);
	}
	this->init_network_node_contexts = original->init_network_node_contexts;
	for (int n_index = 0; n_index < (int)original->init_networks.size(); n_index++) {
		this->init_networks.push_back(new InitNetwork(original->init_networks[n_index]));
	}

	this->next_node_id = original->next_node_id;

	this->average_instances_per_hit = original->average_instances_per_hit;
	this->average_instances_per_run = original->average_instances_per_run;

	this->score_network = new ScoreNetwork(original->score_network);

	this->is_generic = original->is_generic;

	this->ancestor_ids = original->ancestor_ids;
}

void ActionNode::save(ofstream& output_file) {
	output_file << this->action << endl;

	this->action_network->save(output_file);

	this->obs_network->save(output_file);

	output_file << this->init_networks.size() << endl;
	for (int n_index = 0; n_index < (int)this->init_networks.size(); n_index++) {
		output_file << this->init_network_scope_contexts[n_index].size() << endl;
		for (int l_index = 0; l_index < (int)this->init_network_scope_contexts[n_index].size(); l_index++) {
			output_file << this->init_network_scope_contexts[n_index][l_index]->id << endl;
			output_file << this->init_network_node_contexts[n_index][l_index] << endl;
		}

		this->init_networks[n_index]->save(output_file);
	}

	output_file << this->next_node_id << endl;

	output_file << this->average_instances_per_hit << endl;
	output_file << this->average_instances_per_run << endl;

	this->score_network->save(output_file);

	output_file << this->is_generic << endl;

	output_file << this->ancestor_ids.size() << endl;
	for (int a_index = 0; a_index < (int)this->ancestor_ids.size(); a_index++) {
		output_file << this->ancestor_ids[a_index] << endl;
	}
}

void ActionNode::load(ifstream& input_file,
					  Solution* parent_solution) {
	string action_line;
	getline(input_file, action_line);
	this->action = stoi(action_line);

	this->action_network = new ActionNetwork(input_file);

	this->obs_network = new ObsNetwork(input_file);

	string num_init_networks_line;
	getline(input_file, num_init_networks_line);
	int num_init_networks = stoi(num_init_networks_line);
	for (int n_index = 0; n_index < num_init_networks; n_index++) {
		string num_layers_line;
		getline(input_file, num_layers_line);
		int num_layers = stoi(num_layers_line);
		this->init_network_scope_contexts.push_back(vector<Scope*>());
		this->init_network_node_contexts.push_back(vector<int>());
		for (int l_index = 0; l_index < num_layers; l_index++) {
			string scope_id_line;
			getline(input_file, scope_id_line);
			this->init_network_scope_contexts[n_index].push_back(parent_solution->scopes[stoi(scope_id_line)]);

			string node_id_line;
			getline(input_file, node_id_line);
			this->init_network_node_contexts[n_index].push_back(stoi(node_id_line));
		}

		this->init_networks.push_back(new InitNetwork(input_file));
	}

	string next_node_id_line;
	getline(input_file, next_node_id_line);
	this->next_node_id = stoi(next_node_id_line);

	string average_instances_per_hit_line;
	getline(input_file, average_instances_per_hit_line);
	this->average_instances_per_hit = stod(average_instances_per_hit_line);

	string average_instances_per_run_line;
	getline(input_file, average_instances_per_run_line);
	this->average_instances_per_run = stod(average_instances_per_run_line);

	this->score_network = new ScoreNetwork(input_file);

	string is_generic_line;
	getline(input_file, is_generic_line);
	this->is_generic = stoi(is_generic_line);

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
	output_file << this->action << endl;
	output_file << this->next_node_id << endl;
}

ActionNodeHistory::ActionNodeHistory(ActionNode* node) {
	this->node = node;
}

TrainActionNodeHistory::TrainActionNodeHistory(ActionNode* node) {
	this->node = node;

	this->action_network_history = NULL;
	this->obs_network_history = NULL;

	this->score_network_history = NULL;
}

TrainActionNodeHistory::~TrainActionNodeHistory() {
	if (this->action_network_history != NULL) {
		delete this->action_network_history;
	}

	if (this->obs_network_history != NULL) {
		delete this->obs_network_history;
	}

	for (int n_index = 0; n_index < (int)this->init_network_histories.size(); n_index++) {
		if (this->init_network_histories[n_index] != NULL) {
			delete this->init_network_histories[n_index];
		}
	}

	if (this->score_network_history != NULL) {
		delete this->score_network_history;
	}
}
