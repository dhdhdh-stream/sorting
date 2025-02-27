#include "branch_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "factor.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "solution.h"

using namespace std;

BranchNode::BranchNode() {
	this->type = NODE_TYPE_BRANCH;

	this->is_used = false;

	this->experiment = NULL;
}

BranchNode::BranchNode(BranchNode* original,
					   Solution* parent_solution) {
	this->type = NODE_TYPE_BRANCH;

	this->average_val = original->average_val;
	for (int f_index = 0; f_index < (int)original->factors.size(); f_index++) {
		this->factors.push_back(Input(original->factors[f_index],
									  parent_solution));
	}
	this->factor_weights = original->factor_weights;

	this->is_used = original->is_used;

	this->original_next_node_id = original->original_next_node_id;
	this->branch_next_node_id = original->branch_next_node_id;

	this->ancestor_ids = original->ancestor_ids;

	this->experiment = NULL;

	#if defined(MDEBUG) && MDEBUG
	this->verify_key = NULL;
	#endif /* MDEBUG */
}

BranchNode::~BranchNode() {
	if (this->experiment != NULL) {
		this->experiment->decrement(this);
	}
}

#if defined(MDEBUG) && MDEBUG
void BranchNode::clear_verify() {
	this->verify_key = NULL;
	if (this->verify_scores.size() > 0) {
		cout << "seed: " << seed << endl;

		throw invalid_argument("branch node remaining verify");
	}
}
#endif /* MDEBUG */

void BranchNode::clean_inputs(Scope* scope,
							  int node_id) {
	if (scope == this->parent) {
		for (int f_index = (int)this->factors.size()-1; f_index >= 0; f_index--) {
			if (this->factors[f_index].type == INPUT_TYPE_OBS) {
				if (this->factors[f_index].node_context.back() == node_id) {
					this->factors.erase(this->factors.begin() + f_index);
					this->factor_weights.erase(this->factor_weights.begin() + f_index);
				}
			}
		}
	}
}

void BranchNode::save(ofstream& output_file) {
	output_file << this->average_val << endl;
	output_file << this->factors.size() << endl;
	for (int f_index = 0; f_index < (int)this->factors.size(); f_index++) {
		this->factors[f_index].save(output_file);

		output_file << this->factor_weights[f_index] << endl;
	}

	output_file << this->original_next_node_id << endl;
	output_file << this->branch_next_node_id << endl;

	output_file << this->ancestor_ids.size() << endl;
	for (int a_index = 0; a_index < (int)this->ancestor_ids.size(); a_index++) {
		output_file << this->ancestor_ids[a_index] << endl;
	}
}

void BranchNode::load(ifstream& input_file,
					  Solution* parent_solution) {
	string average_val_line;
	getline(input_file, average_val_line);
	this->average_val = stod(average_val_line);

	string num_factors_line;
	getline(input_file, num_factors_line);
	int num_factors = stoi(num_factors_line);
	for (int f_index = 0; f_index < num_factors; f_index++) {
		this->factors.push_back(Input(input_file,
									  parent_solution));

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
}

void BranchNode::link(Solution* parent_solution) {
	for (int f_index = 0; f_index < (int)this->factors.size(); f_index++) {
		if (this->factors[f_index].type == INPUT_TYPE_OBS) {
			ObsNode* obs_node = (ObsNode*)this->parent->nodes[this->factors[f_index].node_context.back()];
			Factor* factor = obs_node->factors[this->factors[f_index].factor_index];

			factor->link(parent_solution);

			obs_node->is_used = true;
		}
	}

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
