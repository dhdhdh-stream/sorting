#include "branch_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "constants.h"
#include "factor.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "solution.h"

using namespace std;

BranchNode::BranchNode() {
	this->type = NODE_TYPE_BRANCH;

	this->is_used = false;

	this->is_init = false;

	this->experiment = NULL;

	this->last_updated_run_index = -1;
 	this->num_measure = 0;
 	this->sum_score = 0.0;

 	this->num_match_measure = 0;
	this->sum_remaining_matches = 0.0;

	#if defined(MDEBUG) && MDEBUG
	this->verify_key = NULL;
	#endif /* MDEBUG */
}

BranchNode::BranchNode(BranchNode* original) {
	this->type = NODE_TYPE_BRANCH;

	this->average_val = original->average_val;
	this->factor_ids = original->factor_ids;
	this->factor_weights = original->factor_weights;

	this->is_used = original->is_used;

	this->original_next_node_id = original->original_next_node_id;
	this->branch_next_node_id = original->branch_next_node_id;

	this->ancestor_ids = original->ancestor_ids;

	this->is_init = true;

	this->experiment = NULL;

	this->last_updated_run_index = -1;
 	this->num_measure = 0;
 	this->sum_score = 0.0;

 	this->num_match_measure = 0;
	this->sum_remaining_matches = 0.0;

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

	this->num_measure = 0;
	this->sum_score = 0.0;

	this->num_match_measure = 0;
	this->sum_remaining_matches = 0.0;
}

void BranchNode::measure_update() {
	this->average_score = this->sum_score / (double)this->num_measure;
	this->average_instances_per_run = (double)this->num_measure / MEASURE_ITERS;
}

void BranchNode::measure_match_update() {
	if (this->num_match_measure == 0) {
		/**
		 * - simply set to 0.0
		 */
		this->average_remaining_matches = 0.0;
	} else {
		this->average_remaining_matches = this->sum_remaining_matches / (double)this->num_match_measure;
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

	output_file << this->average_score << endl;
	output_file << this->average_instances_per_run << endl;

	output_file << this->average_remaining_matches << endl;
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
