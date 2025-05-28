#include "scope_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "constants.h"
#include "globals.h"
#include "scope.h"
#include "solution.h"

using namespace std;

ScopeNode::ScopeNode() {
	this->type = NODE_TYPE_SCOPE;

	this->is_init = false;

	this->experiment = NULL;

	this->last_updated_run_index = -1;
 	this->num_measure = 0;
 	this->sum_score = 0.0;

 	this->num_match_measure = 0;
	this->sum_remaining_matches = 0.0;
}

ScopeNode::ScopeNode(ScopeNode* original,
					 Solution* parent_solution) {
	this->type = NODE_TYPE_SCOPE;

	this->scope = parent_solution->scopes[original->scope->id];

	this->next_node_id = original->next_node_id;

	this->ancestor_ids = original->ancestor_ids;

	this->is_init = true;

	this->experiment = NULL;

	this->last_updated_run_index = -1;
 	this->num_measure = 0;
 	this->sum_score = 0.0;

 	this->num_match_measure = 0;
	this->sum_remaining_matches = 0.0;
}

ScopeNode::~ScopeNode() {
	if (this->experiment != NULL) {
		this->experiment->decrement(this);
	}
}

void ScopeNode::replace_scope(Scope* original_scope,
							  Scope* new_scope) {
	if (this->scope == original_scope) {
		this->scope = new_scope;

		bool has_match = false;
		for (int c_index = 0; c_index < (int)this->parent->child_scopes.size(); c_index++) {
			if (this->parent->child_scopes[c_index] == new_scope) {
				has_match = true;
				break;
			}
		}
		if (!has_match) {
			this->parent->child_scopes.push_back(new_scope);
		}
	}
}

void ScopeNode::clean() {
	if (this->experiment != NULL) {
		this->experiment->decrement(this);
		this->experiment = NULL;
	}

	this->num_measure = 0;
	this->sum_score = 0.0;

	this->num_match_measure = 0;
	this->sum_remaining_matches = 0.0;
}

void ScopeNode::measure_update() {
	this->average_score = this->sum_score / (double)this->num_measure;
	this->average_instances_per_run = (double)this->num_measure / MEASURE_ITERS;
}

void ScopeNode::measure_match_update() {
	if (this->num_match_measure == 0) {
		/**
		 * - simply set to 0.0
		 */
		this->average_remaining_matches = 0.0;
	} else {
		this->average_remaining_matches = this->sum_remaining_matches / (double)this->num_match_measure;
	}
}

void ScopeNode::save(ofstream& output_file) {
	output_file << this->scope->id << endl;

	output_file << this->next_node_id << endl;

	output_file << this->ancestor_ids.size() << endl;
	for (int a_index = 0; a_index < (int)this->ancestor_ids.size(); a_index++) {
		output_file << this->ancestor_ids[a_index] << endl;
	}

	output_file << this->average_score << endl;
	output_file << this->average_instances_per_run << endl;

	output_file << this->average_remaining_matches << endl;
}

void ScopeNode::load(ifstream& input_file,
					 Solution* parent_solution) {
	string scope_id_line;
	getline(input_file, scope_id_line);
	this->scope = parent_solution->scopes[stoi(scope_id_line)];

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

	this->is_init = true;

	string average_score_line;
	getline(input_file, average_score_line);
	this->average_score = stod(average_score_line);

	string average_instances_per_run_line;
	getline(input_file, average_instances_per_run_line);
	this->average_instances_per_run = stod(average_instances_per_run_line);

	string average_remaining_matches_line;
	getline(input_file, average_remaining_matches_line);
	this->average_remaining_matches = stod(average_remaining_matches_line);
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
