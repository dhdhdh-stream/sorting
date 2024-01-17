#include "branch_node.h"

#include <iostream>

#include "globals.h"
#include "retrain_branch_experiment.h"
#include "scope.h"
#include "solution.h"
#include "state.h"

using namespace std;

BranchNode::BranchNode() {
	this->type = NODE_TYPE_BRANCH;

	this->experiment = NULL;

	#if defined(MDEBUG) && MDEBUG
	this->verify_key = NULL;
	#endif /* MDEBUG */

	this->is_potential = false;
}

BranchNode::~BranchNode() {
	if (this->experiment != NULL) {
		delete this->experiment;
	}
}

#if defined(MDEBUG) && MDEBUG
void BranchNode::clear_verify() {
	this->verify_key = NULL;
	if (this->verify_original_scores.size() > 0
			|| this->verify_branch_scores.size() > 0
			|| this->verify_factors.size() > 0) {
		throw invalid_argument("branch node remaining verify");
	}
}
#endif /* MDEBUG */

void BranchNode::success_reset() {
	if (this->experiment != NULL) {
		delete this->experiment;
		this->experiment = NULL;
	}
}

void BranchNode::fail_reset() {
	if (this->experiment != NULL) {
		delete this->experiment;
		this->experiment = NULL;
	}
}

void BranchNode::save(ofstream& output_file) {
	output_file << this->branch_scope_context.size() << endl;
	for (int c_index = 0; c_index < (int)this->branch_scope_context.size(); c_index++) {
		output_file << this->branch_scope_context[c_index] << endl;
		output_file << this->branch_node_context[c_index] << endl;
	}

	output_file << this->branch_is_pass_through << endl;

	output_file << this->original_score_mod << endl;
	output_file << this->branch_score_mod << endl;

	output_file << this->decision_state_is_local.size() << endl;
	for (int s_index = 0; s_index < (int)this->decision_state_is_local.size(); s_index++) {
		output_file << this->decision_state_is_local[s_index] << endl;
		output_file << this->decision_state_indexes[s_index] << endl;
		output_file << this->decision_original_weights[s_index] << endl;
		output_file << this->decision_branch_weights[s_index] << endl;
	}

	output_file << this->decision_standard_deviation << endl;

	output_file << this->original_next_node_id << endl;
	output_file << this->branch_next_node_id << endl;
}

void BranchNode::load(ifstream& input_file) {
	string branch_scope_context_size_line;
	getline(input_file, branch_scope_context_size_line);
	int branch_scope_context_size = stoi(branch_scope_context_size_line);
	for (int c_index = 0; c_index < branch_scope_context_size; c_index++) {
		string scope_context_line;
		getline(input_file, scope_context_line);
		this->branch_scope_context.push_back(stoi(scope_context_line));

		string node_context_line;
		getline(input_file, node_context_line);
		this->branch_node_context.push_back(stoi(node_context_line));
	}

	string branch_is_pass_through_line;
	getline(input_file, branch_is_pass_through_line);
	this->branch_is_pass_through = stoi(branch_is_pass_through_line);

	string original_score_mod_line;
	getline(input_file, original_score_mod_line);
	this->original_score_mod = stod(original_score_mod_line);

	string branch_score_mod_line;
	getline(input_file, branch_score_mod_line);
	this->branch_score_mod = stod(branch_score_mod_line);

	string decision_state_is_local_size_line;
	getline(input_file, decision_state_is_local_size_line);
	int decision_state_is_local_size = stoi(decision_state_is_local_size_line);
	for (int s_index = 0; s_index < decision_state_is_local_size; s_index++) {
		string is_local_line;
		getline(input_file, is_local_line);
		this->decision_state_is_local.push_back(stoi(is_local_line));

		string index_line;
		getline(input_file, index_line);
		this->decision_state_indexes.push_back(stoi(index_line));

		string original_weight_line;
		getline(input_file, original_weight_line);
		this->decision_original_weights.push_back(stod(original_weight_line));

		string branch_weight_line;
		getline(input_file, branch_weight_line);
		this->decision_branch_weights.push_back(stod(branch_weight_line));
	}

	string decision_standard_deviation_line;
	getline(input_file, decision_standard_deviation_line);
	this->decision_standard_deviation = stod(decision_standard_deviation_line);

	string original_next_node_id_line;
	getline(input_file, original_next_node_id_line);
	this->original_next_node_id = stoi(original_next_node_id_line);

	string branch_next_node_id_line;
	getline(input_file, branch_next_node_id_line);
	this->branch_next_node_id = stoi(branch_next_node_id_line);
}

void BranchNode::link() {
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
	if (this->branch_scope_context.size() == 0) {
		output_file << -1 << endl;
	} else {
		output_file << this->branch_scope_context[0] << endl;
	}

	output_file << this->branch_is_pass_through << endl;

	output_file << this->original_next_node_id << endl;
	output_file << this->branch_next_node_id << endl;
}

BranchNodeHistory::BranchNodeHistory(BranchNode* node) {
	this->node = node;
}

BranchNodeHistory::BranchNodeHistory(BranchNodeHistory* original) {
	this->node = original->node;
}
