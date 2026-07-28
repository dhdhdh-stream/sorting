#include "branch_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "constants.h"
#include "globals.h"
#include "init_network.h"
#include "scope.h"
#include "score_network.h"
#include "solution.h"

using namespace std;

BranchNode::BranchNode() {
	this->type = NODE_TYPE_BRANCH;

	this->original_average_instances_per_hit = 1.0;
	this->original_average_instances_per_run = 0.0;
	this->original_experiment = NULL;
	this->branch_average_instances_per_hit = 1.0;
	this->branch_average_instances_per_run = 0.0;
	this->branch_experiment = NULL;

	this->original_curr_num_instances = 0;
	this->branch_curr_num_instances = 0;
}

BranchNode::~BranchNode() {
	for (int n_index = 0; n_index < (int)this->init_networks.size(); n_index++) {
		delete this->init_networks[n_index];
	}

	delete this->original_network;
	delete this->branch_network;

	if (this->original_experiment != NULL) {
		delete this->original_experiment;
	}
	if (this->branch_experiment != NULL) {
		delete this->branch_experiment;
	}
}

void BranchNode::copy_from(BranchNode* original,
						   Solution* parent_solution) {
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

	this->original_network = new ScoreNetwork(original->original_network);
	this->branch_network = new ScoreNetwork(original->branch_network);

	this->original_next_node_id = original->original_next_node_id;
	this->branch_next_node_id = original->branch_next_node_id;

	this->is_ramp = original->is_ramp;

	this->consec_original = original->consec_original;
	this->consec_branch = original->consec_branch;

	this->original_average_instances_per_hit = original->original_average_instances_per_hit;
	this->original_average_instances_per_run = original->original_average_instances_per_run;
	this->branch_average_instances_per_hit = original->branch_average_instances_per_hit;
	this->branch_average_instances_per_run = original->branch_average_instances_per_run;

	this->ancestor_ids = original->ancestor_ids;
}

void BranchNode::save(ofstream& output_file) {
	output_file << this->init_networks.size() << endl;
	for (int n_index = 0; n_index < (int)this->init_networks.size(); n_index++) {
		output_file << this->init_network_scope_contexts[n_index].size() << endl;
		for (int l_index = 0; l_index < (int)this->init_network_scope_contexts[n_index].size(); l_index++) {
			output_file << this->init_network_scope_contexts[n_index][l_index]->id << endl;
			output_file << this->init_network_node_contexts[n_index][l_index] << endl;
		}

		this->init_networks[n_index]->save(output_file);
	}

	this->original_network->save(output_file);
	this->branch_network->save(output_file);

	output_file << this->original_next_node_id << endl;
	output_file << this->branch_next_node_id << endl;

	output_file << this->is_ramp << endl;

	output_file << this->consec_original << endl;
	output_file << this->consec_branch << endl;

	output_file << this->original_average_instances_per_hit << endl;
	output_file << this->original_average_instances_per_run << endl;
	output_file << this->branch_average_instances_per_hit << endl;
	output_file << this->branch_average_instances_per_run << endl;

	output_file << this->ancestor_ids.size() << endl;
	for (int a_index = 0; a_index < (int)this->ancestor_ids.size(); a_index++) {
		output_file << this->ancestor_ids[a_index] << endl;
	}
}

void BranchNode::load(ifstream& input_file,
					  Solution* parent_solution) {
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

	this->original_network = new ScoreNetwork(input_file);
	this->branch_network = new ScoreNetwork(input_file);

	string original_next_node_id_line;
	getline(input_file, original_next_node_id_line);
	this->original_next_node_id = stoi(original_next_node_id_line);

	string branch_next_node_id_line;
	getline(input_file, branch_next_node_id_line);
	this->branch_next_node_id = stoi(branch_next_node_id_line);

	string is_ramp_line;
	getline(input_file, is_ramp_line);
	this->is_ramp = stoi(is_ramp_line);

	string consec_original_line;
	getline(input_file, consec_original_line);
	this->consec_original = stoi(consec_original_line);

	// temp
	if (this->consec_original >= CONSEC_DEPRECATE_LIMIT) {
		cout << "this->consec_original >= CONSEC_DEPRECATE_LIMIT" << endl;
	}

	string consec_branch_line;
	getline(input_file, consec_branch_line);
	this->consec_branch = stoi(consec_branch_line);

	// temp
	if (this->consec_branch >= CONSEC_DEPRECATE_LIMIT) {
		cout << "this->consec_branch >= CONSEC_DEPRECATE_LIMIT" << endl;
	}

	string original_average_instances_per_hit_line;
	getline(input_file, original_average_instances_per_hit_line);
	this->original_average_instances_per_hit = stod(original_average_instances_per_hit_line);

	string original_average_instances_per_run_line;
	getline(input_file, original_average_instances_per_run_line);
	this->original_average_instances_per_run = stod(original_average_instances_per_run_line);

	string branch_average_instances_per_hit_line;
	getline(input_file, branch_average_instances_per_hit_line);
	this->branch_average_instances_per_hit = stod(branch_average_instances_per_hit_line);

	string branch_average_instances_per_run_line;
	getline(input_file, branch_average_instances_per_run_line);
	this->branch_average_instances_per_run = stod(branch_average_instances_per_run_line);

	string num_ancestors_line;
	getline(input_file, num_ancestors_line);
	int num_ancestors = stoi(num_ancestors_line);
	for (int a_index = 0; a_index < num_ancestors; a_index++) {
		string ancestor_id_line;
		getline(input_file, ancestor_id_line);
		this->ancestor_ids.push_back(stoi(ancestor_id_line));
	}
}

void BranchNode::link(Solution* parent_solution) {
	if (this->original_next_node_id == -1) {
		this->original_next_node = NULL;
	} else {
		this->original_next_node = this->parent->nodes[this->original_next_node_id];
	}

	if (this->branch_next_node_id == -1) {
		this->branch_next_node = NULL;
	} else {
		this->branch_next_node = this->parent->nodes[this->branch_next_node_id];
	}
}

void BranchNode::save_for_display(ofstream& output_file) {
	output_file << this->original_next_node_id << endl;
	output_file << this->branch_next_node_id << endl;
}

BranchNodeHistory::BranchNodeHistory(BranchNode* node) {
	this->node = node;
}
