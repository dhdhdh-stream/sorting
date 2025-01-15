#include "obs_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "factor.h"
#include "scope.h"

using namespace std;

ObsNode::ObsNode() {
	this->type = NODE_TYPE_OBS;

	this->average_instances_per_run = 0.0;

	this->was_commit = false;

	this->num_measure = 0;
	this->sum_score = 0.0;
}

ObsNode::ObsNode(ObsNode* original,
				 Solution* parent_solution) {
	this->type = NODE_TYPE_OBS;

	this->is_used = original->is_used;

	for (int f_index = 0; f_index < (int)original->factors.size(); f_index++) {
		Factor* factor = new Factor(original->factors[f_index],
									parent_solution);
		this->factors.push_back(factor);
	}

	this->next_node_id = original->next_node_id;

	this->ancestor_ids = original->ancestor_ids;

	this->average_instances_per_run = 0.0;

	this->was_commit = false;

	this->num_measure = 0;
	this->sum_score = 0.0;
}

ObsNode::~ObsNode() {
	for (int f_index = 0; f_index < (int)this->factors.size(); f_index++) {
		delete this->factors[f_index];
	}

	for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
		this->experiments[e_index]->decrement(this);
	}
}

void ObsNode::clean_inputs(Scope* scope,
						   int node_id) {
	for (int f_index = 0; f_index < (int)this->factors.size(); f_index++) {
		this->factors[f_index]->clean_inputs(scope,
											 node_id);
	}
}

void ObsNode::clean_inputs(Scope* scope) {
	for (int f_index = 0; f_index < (int)this->factors.size(); f_index++) {
		this->factors[f_index]->clean_inputs(scope);
	}
}

void ObsNode::clear_experiments() {
	for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
		this->experiments[e_index]->decrement(this);
	}
	this->experiments.clear();
}

void ObsNode::save(ofstream& output_file) {
	output_file << this->factors.size() << endl;
	for (int f_index = 0; f_index < (int)this->factors.size(); f_index++) {
		this->factors[f_index]->save(output_file);
	}

	output_file << this->next_node_id << endl;

	output_file << this->ancestor_ids.size() << endl;
	for (int a_index = 0; a_index < (int)this->ancestor_ids.size(); a_index++) {
		output_file << this->ancestor_ids[a_index] << endl;
	}

	output_file << this->average_instances_per_run << endl;

	output_file << this->was_commit << endl;
}

void ObsNode::load(ifstream& input_file,
				   Solution* parent_solution) {
	this->is_used = false;

	string num_factors_line;
	getline(input_file, num_factors_line);
	int num_factors = stoi(num_factors_line);
	for (int f_index = 0; f_index < num_factors; f_index++) {
		Factor* factor = new Factor();
		factor->load(input_file,
					 parent_solution);
		this->factors.push_back(factor);
	}

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

	string average_instances_per_run_line;
	getline(input_file, average_instances_per_run_line);
	this->average_instances_per_run = stod(average_instances_per_run_line);

	string was_commit_line;
	getline(input_file, was_commit_line);
	this->was_commit = stoi(was_commit_line);
}

void ObsNode::link(Solution* parent_solution) {
	for (int f_index = 0; f_index < (int)this->factors.size(); f_index++) {
		this->factors[f_index]->link(parent_solution);
	}

	if (this->next_node_id == -1) {
		this->next_node = NULL;
	} else {
		this->next_node = this->parent->nodes[this->next_node_id];
	}
}

void ObsNode::save_for_display(ofstream& output_file) {
	output_file << this->next_node_id << endl;
}

ObsNodeHistory::ObsNodeHistory(ObsNode* node) {
	this->node = node;
}
