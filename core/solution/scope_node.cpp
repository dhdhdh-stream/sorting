#include "scope_node.h"

#include <iostream>

#include "branch_experiment.h"
#include "globals.h"
#include "pass_through_experiment.h"
#include "scope.h"
#include "solution.h"
#include "state.h"

using namespace std;

ScopeNode::ScopeNode() {
	this->type = NODE_TYPE_SCOPE;

	this->id = -1;

	this->experiment = NULL;

	this->is_potential = false;

	#if defined(MDEBUG) && MDEBUG
	this->verify_key = NULL;
	#endif /* MDEBUG */
}

ScopeNode::~ScopeNode() {
	if (this->experiment != NULL) {
		delete this->experiment;
	}
}

void ScopeNode::success_reset() {
	if (this->experiment != NULL) {
		delete this->experiment;
		this->experiment = NULL;
	}
}

void ScopeNode::fail_reset() {
	if (this->experiment != NULL) {
		delete this->experiment;
		this->experiment = NULL;
	}
}

#if defined(MDEBUG) && MDEBUG
void ScopeNode::clear_verify() {
	this->verify_key = NULL;
	if (this->verify_input_input_state_vals.size() > 0
			|| this->verify_input_local_state_vals.size() > 0
			|| this->verify_output_input_state_vals.size() > 0
			|| this->verify_output_local_state_vals.size() > 0) {
		cout << "this->verify_input_input_state_vals.size(): " << this->verify_input_input_state_vals.size() << endl;
		cout << "this->verify_input_local_state_vals.size(): " << this->verify_input_local_state_vals.size() << endl;
		cout << "this->verify_output_input_state_vals.size(): " << this->verify_output_input_state_vals.size() << endl;
		cout << "this->verify_output_local_state_vals.size(): " << this->verify_output_local_state_vals.size() << endl;

		throw invalid_argument("scope node remaining verify");
	}
}
#endif /* MDEBUG */

void ScopeNode::save(ofstream& output_file) {
	output_file << this->inner_scope->id << endl;

	output_file << this->input_types.size() << endl;
	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		output_file << this->input_types[i_index] << endl;
		output_file << this->input_inner_is_local[i_index] << endl;
		output_file << this->input_inner_indexes[i_index] << endl;
		output_file << this->input_outer_is_local[i_index] << endl;
		output_file << this->input_outer_indexes[i_index] << endl;
		output_file << this->input_init_vals[i_index] << endl;
	}

	output_file << this->output_inner_indexes.size() << endl;
	for (int o_index = 0; o_index < (int)this->output_inner_indexes.size(); o_index++) {
		output_file << this->output_inner_is_local[o_index] << endl;
		output_file << this->output_inner_indexes[o_index] << endl;
		output_file << this->output_outer_is_local[o_index] << endl;
		output_file << this->output_outer_indexes[o_index] << endl;
	}

	output_file << this->next_node_id << endl;
}

void ScopeNode::load(ifstream& input_file) {
	string inner_scope_id_line;
	getline(input_file, inner_scope_id_line);
	this->inner_scope = solution->scopes[stoi(inner_scope_id_line)];

	string num_inputs_line;
	getline(input_file, num_inputs_line);
	int num_inputs = stoi(num_inputs_line);
	for (int i_index = 0; i_index < num_inputs; i_index++) {
		string type_line;
		getline(input_file, type_line);
		this->input_types.push_back(stoi(type_line));

		string inner_is_local_line;
		getline(input_file, inner_is_local_line);
		this->input_inner_is_local.push_back(stoi(inner_is_local_line));

		string inner_index_line;
		getline(input_file, inner_index_line);
		this->input_inner_indexes.push_back(stoi(inner_index_line));

		string outer_is_local_line;
		getline(input_file, outer_is_local_line);
		this->input_outer_is_local.push_back(stoi(outer_is_local_line));

		string outer_index_line;
		getline(input_file, outer_index_line);
		this->input_outer_indexes.push_back(stoi(outer_index_line));

		string init_val_line;
		getline(input_file, init_val_line);
		this->input_init_vals.push_back(stod(init_val_line));
	}

	string num_outputs_line;
	getline(input_file, num_outputs_line);
	int num_outputs = stoi(num_outputs_line);
	for (int o_index = 0; o_index < num_outputs; o_index++) {
		string inner_is_local_line;
		getline(input_file, inner_is_local_line);
		this->output_inner_is_local.push_back(stoi(inner_is_local_line));

		string inner_index_line;
		getline(input_file, inner_index_line);
		this->output_inner_indexes.push_back(stoi(inner_index_line));

		string outer_is_local_line;
		getline(input_file, outer_is_local_line);
		this->output_outer_is_local.push_back(stoi(outer_is_local_line));

		string outer_index_line;
		getline(input_file, outer_index_line);
		this->output_outer_indexes.push_back(stoi(outer_index_line));
	}

	string next_node_id_line;
	getline(input_file, next_node_id_line);
	this->next_node_id = stoi(next_node_id_line);
}

void ScopeNode::link() {
	if (this->next_node_id == -1) {
		this->next_node = NULL;
	} else {
		this->next_node = this->parent->nodes[this->next_node_id];
	}
}

void ScopeNode::save_for_display(ofstream& output_file) {
	output_file << this->inner_scope->id << endl;

	output_file << this->next_node_id << endl;
}

ScopeNodeHistory::ScopeNodeHistory(ScopeNode* node) {
	this->node = node;

	this->experiment_history = NULL;
}

ScopeNodeHistory::ScopeNodeHistory(ScopeNodeHistory* original) {
	this->node = original->node;

	this->inner_scope_history = new ScopeHistory(original->inner_scope_history);

	if (original->experiment_history != NULL) {
		if (original->experiment_history->experiment->type == EXPERIMENT_TYPE_BRANCH) {
			BranchExperimentInstanceHistory* branch_experiment_history = (BranchExperimentInstanceHistory*)original->experiment_history;
			this->experiment_history = new BranchExperimentInstanceHistory(branch_experiment_history);
		} else {
			// original->experiment_history->experiment->type == EXPERIMENT_TYPE_PASS_THROUGH
			PassThroughExperimentInstanceHistory* pass_through_experiment_history = (PassThroughExperimentInstanceHistory*)original->experiment_history;
			this->experiment_history = new PassThroughExperimentInstanceHistory(pass_through_experiment_history);
		}
	} else {
		this->experiment_history = NULL;
	}
}

ScopeNodeHistory::~ScopeNodeHistory() {
	delete this->inner_scope_history;

	if (this->experiment_history != NULL) {
		delete this->experiment_history;
	}
}
