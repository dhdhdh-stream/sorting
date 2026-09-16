#include "scope_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "constants.h"
#include "globals.h"
#include "init_network.h"
#include "pass_through_network.h"
#include "scope.h"
#include "score_network.h"
#include "solution.h"
#include "transition_network.h"

using namespace std;

ScopeNode::ScopeNode() {
	this->type = NODE_TYPE_SCOPE;

	this->average_instances_per_hit = 1.0;
	this->average_instances_per_run = 0.0;
	this->experiment = NULL;

	this->curr_num_instances = 0;
}

ScopeNode::~ScopeNode() {
	for (int n_index = 0; n_index < (int)this->in_pass_through_networks.size(); n_index++) {
		delete this->in_pass_through_networks[n_index];
	}
	delete this->in_network;

	for (int n_index = 0; n_index < (int)this->out_pass_through_networks.size(); n_index++) {
		delete this->out_pass_through_networks[n_index];
	}
	delete this->out_network;

	if (this->experiment != NULL) {
		delete this->experiment;
	}
}

void ScopeNode::copy_from(ScopeNode* original,
						  Solution* parent_solution) {
	for (int n_index = 0; n_index < (int)original->in_pass_through_networks.size(); n_index++) {
		this->in_pass_through_networks.push_back(new PassThroughNetwork(original->in_pass_through_networks[n_index]));
	}
	this->in_network = new TransitionNetwork(original->in_network);

	this->scope = parent_solution->scopes[original->scope->id];

	for (int n_index = 0; n_index < (int)original->out_pass_through_networks.size(); n_index++) {
		this->out_pass_through_networks.push_back(new PassThroughNetwork(original->out_pass_through_networks[n_index]));
	}
	this->out_network = new TransitionNetwork(original->out_network);

	this->next_node_id = original->next_node_id;

	this->average_instances_per_hit = original->average_instances_per_hit;
	this->average_instances_per_run = original->average_instances_per_run;

	this->ancestor_ids = original->ancestor_ids;
}

void ScopeNode::save(ofstream& output_file) {
	output_file << this->in_pass_through_networks.size() << endl;
	for (int n_index = 0; n_index < (int)this->in_pass_through_networks.size(); n_index++) {
		this->in_pass_through_networks[n_index]->save(output_file);
	}
	this->in_network->save(output_file);

	output_file << this->scope->id << endl;

	output_file << this->out_pass_through_networks.size() << endl;
	for (int n_index = 0; n_index < (int)this->out_pass_through_networks.size(); n_index++) {
		this->out_pass_through_networks[n_index]->save(output_file);
	}
	this->out_network->save(output_file);

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
	string in_pass_through_networks_size_line;
	getline(input_file, in_pass_through_networks_size_line);
	int in_pass_through_networks_size = stoi(in_pass_through_networks_size_line);
	for (int n_index = 0; n_index < in_pass_through_networks_size; n_index++) {
		this->in_pass_through_networks.push_back(new PassThroughNetwork(input_file));
	}

	this->in_network = new TransitionNetwork(input_file);

	string scope_id_line;
	getline(input_file, scope_id_line);
	this->scope = parent_solution->scopes[stoi(scope_id_line)];

	string out_pass_through_networks_size_line;
	getline(input_file, out_pass_through_networks_size_line);
	int out_pass_through_networks_size = stoi(out_pass_through_networks_size_line);
	for (int n_index = 0; n_index < out_pass_through_networks_size; n_index++) {
		this->out_pass_through_networks.push_back(new PassThroughNetwork(input_file));
	}

	this->out_network = new TransitionNetwork(input_file);

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

TrainScopeNodeHistory::TrainScopeNodeHistory(ScopeNode* node) {
	this->node = node;

	this->in_network_history = NULL;

	this->out_network_history = NULL;
}

TrainScopeNodeHistory::~TrainScopeNodeHistory() {
	if (this->in_network_history != NULL) {
		delete this->in_network_history;
	}

	delete this->scope_history;

	if (this->out_network_history != NULL) {
		delete this->out_network_history;
	}
}
