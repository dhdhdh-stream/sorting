#include "scope_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "constants.h"
#include "globals.h"
#include "init_network.h"
#include "scope.h"
#include "score_network.h"
#include "solution.h"

using namespace std;

ScopeNode::ScopeNode() {
	this->type = NODE_TYPE_SCOPE;

	this->average_instances_per_hit = 1.0;
	this->average_instances_per_run = 0.0;
	this->experiment = NULL;

	this->curr_num_instances = 0;
}

ScopeNode::~ScopeNode() {
	for (int n_index = 0; n_index < (int)this->init_networks.size(); n_index++) {
		delete this->init_networks[n_index];
	}
	for (int n_index = 0; n_index < (int)this->prev_init_networks.size(); n_index++) {
		delete this->prev_init_networks[n_index];
	}

	delete this->score_network;

	delete this->explore_score_network;

	if (this->experiment != NULL) {
		delete this->experiment;
	}
}

void ScopeNode::save(ofstream& output_file) {
	output_file << this->scope->id << endl;

	output_file << this->init_networks.size() << endl;
	for (int n_index = 0; n_index < (int)this->init_networks.size(); n_index++) {
		output_file << this->init_network_scope_contexts[n_index].size() << endl;
		for (int l_index = 0; l_index < (int)this->init_network_scope_contexts[n_index].size(); l_index++) {
			output_file << this->init_network_scope_contexts[n_index][l_index]->id << endl;
			output_file << this->init_network_node_contexts[n_index][l_index] << endl;
		}

		this->init_networks[n_index]->save(output_file);
		this->prev_init_networks[n_index]->save(output_file);
	}

	this->score_network->save(output_file);

	this->explore_score_network->save(output_file);

	output_file << this->next_node_id << endl;

	output_file << this->average_instances_per_hit << endl;
	output_file << this->average_instances_per_run << endl;

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
		this->prev_init_networks.push_back(new InitNetwork(input_file));
	}

	this->score_network = new ScoreNetwork(input_file);

	this->explore_score_network = new ScoreNetwork(input_file);

	string next_node_id_line;
	getline(input_file, next_node_id_line);
	this->next_node_id = stoi(next_node_id_line);

	string average_instances_per_hit_line;
	getline(input_file, average_instances_per_hit_line);
	this->average_instances_per_hit = stod(average_instances_per_hit_line);

	string average_instances_per_run_line;
	getline(input_file, average_instances_per_run_line);
	this->average_instances_per_run = stod(average_instances_per_run_line);

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
