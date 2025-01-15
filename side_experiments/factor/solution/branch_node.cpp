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

	this->average_instances_per_run = 0.0;

	this->was_commit = false;

	this->num_measure = 0;
	this->sum_score = 0.0;
}

BranchNode::BranchNode(BranchNode* original) {
	this->type = NODE_TYPE_BRANCH;

	this->average_val = original->average_val;
	this->factor_ids = original->factor_ids;
	this->factor_weights = original->factor_weights;

	this->original_next_node_id = original->original_next_node_id;
	this->branch_next_node_id = original->branch_next_node_id;

	this->ancestor_ids = original->ancestor_ids;

	this->average_instances_per_run = 0.0;

	this->was_commit = false;

	this->num_measure = 0;
	this->sum_score = 0.0;

	#if defined(MDEBUG) && MDEBUG
	this->verify_key = NULL;
	#endif /* MDEBUG */
}

BranchNode::~BranchNode() {
	for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
		this->experiments[e_index]->decrement(this);
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
		for (int f_index = (int)this->factor_ids.size()-1; f_index >= 0; f_index--) {
			if (this->factor_ids[f_index].first == node_id) {
				this->factor_ids.erase(this->factor_ids.begin() + f_index);
				this->factor_weights.erase(this->factor_weights.begin() + f_index);
			}
		}
	}

	for (int i_index = (int)this->input_scope_contexts.size()-1; i_index >= 0; i_index--) {
		bool is_match = false;
		for (int l_index = 0; l_index < (int)this->input_scope_contexts[i_index].size(); l_index++) {
			if (this->input_scope_contexts[i_index][l_index] == scope
					&& this->input_node_context_ids[i_index][l_index] == node_id) {
				is_match = true;
				break;
			}
		}

		if (is_match) {
			this->input_scope_contexts.erase(this->input_scope_contexts.begin() + i_index);
			this->input_node_context_ids.erase(this->input_node_context_ids.begin() + i_index);
		}
	}
}

void BranchNode::clean_inputs(Scope* scope) {
	for (int i_index = (int)this->input_scope_contexts.size()-1; i_index >= 0; i_index--) {
		bool is_match = false;
		for (int l_index = 0; l_index < (int)this->input_scope_contexts[i_index].size(); l_index++) {
			if (this->input_scope_contexts[i_index][l_index] == scope) {
				is_match = true;
				break;
			}
		}

		if (is_match) {
			this->input_scope_contexts.erase(this->input_scope_contexts.begin() + i_index);
			this->input_node_context_ids.erase(this->input_node_context_ids.begin() + i_index);
		}
	}
}

void BranchNode::clear_experiments() {
	for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
		this->experiments[e_index]->decrement(this);
	}
	this->experiments.clear();
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

	output_file << this->average_instances_per_run << endl;

	output_file << this->was_commit << endl;
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

	string average_instances_per_run_line;
	getline(input_file, average_instances_per_run_line);
	this->average_instances_per_run = stod(average_instances_per_run_line);

	string was_commit_line;
	getline(input_file, was_commit_line);
	this->was_commit = stoi(was_commit_line);
}

void BranchNode::link(Solution* parent_solution) {
	for (int f_index = 0; f_index < (int)this->factor_ids.size(); f_index++) {
		ObsNode* obs_node = (ObsNode*)this->parent->nodes[this->factor_ids[f_index].first];
		Factor* factor = obs_node->factors[this->factor_ids[f_index].second];

		factor->link(parent_solution);

		vector<Scope*> scope_context{this->parent};
		vector<int> node_context_ids{this->factor_ids[f_index].first};

		bool is_existing = false;
		for (int i_index = 0; i_index < (int)factor->input_scope_contexts.size(); i_index++) {
			if (factor->input_scope_contexts[i_index] == scope_context
					&& factor->input_node_context_ids[i_index] == node_context_ids) {
				is_existing = true;
				break;
			}
		}
		if (!is_existing) {
			factor->input_scope_contexts.push_back(scope_context);
			factor->input_node_context_ids.push_back(node_context_ids);
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
