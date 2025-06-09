#include "branch_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "confusion.h"
#include "constants.h"
#include "factor.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "solution.h"

using namespace std;

BranchNode::BranchNode() {
	this->type = NODE_TYPE_BRANCH;

	this->is_init = false;

	this->experiment = NULL;
	this->confusion = NULL;
}

BranchNode::~BranchNode() {
	if (this->experiment != NULL) {
		this->experiment->decrement(this);
	}

	if (this->confusion != NULL) {
		delete this->confusion;
	}
}

void BranchNode::clean_inputs(Scope* scope,
							  int node_id) {
	if (scope == this->parent) {
		for (int f_index = (int)this->factor_ids.size()-1; f_index >= 0; f_index--) {
			if (this->factor_ids[f_index].first == node_id) {
				this->factor_ids.erase(this->factor_ids.begin() + f_index);
				this->factor_weights.erase(this->factor_weights.begin() + f_index);
			}
		}
	}
}

void BranchNode::replace_factor(Scope* scope,
								int original_node_id,
								int original_factor_index,
								int new_node_id,
								int new_factor_index) {
	if (this->parent == scope) {
		for (int f_index = 0; f_index < (int)this->factor_ids.size(); f_index++) {
			if (this->factor_ids[f_index].first == original_node_id
					&& this->factor_ids[f_index].second == original_factor_index) {
				this->factor_ids[f_index].first = new_node_id;
				this->factor_ids[f_index].second = new_factor_index;
			}
		}
	}
}

void BranchNode::clean() {
	if (this->experiment != NULL) {
		this->experiment->decrement(this);
		this->experiment = NULL;
	}

	if (this->confusion != NULL) {
		delete this->confusion;
		this->confusion = NULL;
	}
}

void BranchNode::save(ofstream& output_file) {
	output_file << this->average_val << endl;
	output_file << this->factor_ids.size() << endl;
	for (int f_index = 0; f_index < (int)this->factor_ids.size(); f_index++) {
		output_file << this->factor_ids[f_index].first << endl;
		output_file << this->factor_ids[f_index].second << endl;
		output_file << this->factor_weights[f_index] << endl;
	}

	output_file << this->original_next_node_id << endl;
	output_file << this->branch_next_node_id << endl;

	output_file << this->ancestor_ids.size() << endl;
	for (int a_index = 0; a_index < (int)this->ancestor_ids.size(); a_index++) {
		output_file << this->ancestor_ids[a_index] << endl;
	}
}

void BranchNode::load(ifstream& input_file) {
	string average_val_line;
	getline(input_file, average_val_line);
	this->average_val = stod(average_val_line);

	string num_factors_line;
	getline(input_file, num_factors_line);
	int num_factors = stoi(num_factors_line);
	for (int f_index = 0; f_index < num_factors; f_index++) {
		string node_id_line;
		getline(input_file, node_id_line);
		int node_id = stoi(node_id_line);

		string factor_id_line;
		getline(input_file, factor_id_line);
		int factor_id = stoi(factor_id_line);

		this->factor_ids.push_back({node_id, factor_id});

		string weight_line;
		getline(input_file, weight_line);
		this->factor_weights.push_back(stod(weight_line));
	}

	string original_next_node_id_line;
	getline(input_file, original_next_node_id_line);
	this->original_next_node_id = stoi(original_next_node_id_line);

	string branch_next_node_id_line;
	getline(input_file, branch_next_node_id_line);
	this->branch_next_node_id = stoi(branch_next_node_id_line);

	string num_ancestors_line;
	getline(input_file, num_ancestors_line);
	int num_ancestors = stoi(num_ancestors_line);
	for (int a_index = 0; a_index < num_ancestors; a_index++) {
		string ancestor_id_line;
		getline(input_file, ancestor_id_line);
		this->ancestor_ids.push_back(stoi(ancestor_id_line));
	}

	this->is_init = true;
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
