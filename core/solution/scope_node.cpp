#include "scope_node.h"

#include <iostream>

#include "branch_experiment.h"
#include "globals.h"
#include "scope.h"
#include "solution.h"
#include "state.h"

using namespace std;

ScopeNode::ScopeNode() {
	this->type = NODE_TYPE_SCOPE;

	this->id = -1;

	this->experiment = NULL;
}

ScopeNode::ScopeNode(ifstream& input_file,
					 int id) {
	this->type = NODE_TYPE_SCOPE;

	this->id = id;

	string inner_scope_id_line;
	getline(input_file, inner_scope_id_line);
	this->inner_scope = solution->scopes[stoi(inner_scope_id_line)];

	string starting_node_ids_size_line;
	getline(input_file, starting_node_ids_size_line);
	int starting_node_ids_size = stoi(starting_node_ids_size_line);
	for (int l_index = 0; l_index < starting_node_ids_size; l_index++) {
		string node_id_line;
		getline(input_file, node_id_line);
		this->starting_node_ids.push_back(stoi(node_id_line));
	}

	string num_inputs_line;
	getline(input_file, num_inputs_line);
	int num_inputs = stoi(num_inputs_line);
	for (int i_index = 0; i_index < num_inputs; i_index++) {
		string type_line;
		getline(input_file, type_line);
		this->input_types.push_back(stoi(type_line));

		string inner_layer_line;
		getline(input_file, inner_layer_line);
		this->input_inner_layers.push_back(stoi(inner_layer_line));

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

	string state_defs_size_line;
	getline(input_file, state_defs_size_line);
	int state_defs_size = stoi(state_defs_size_line);
	for (int s_index = 0; s_index < state_defs_size; s_index++) {
		string is_local_line;
		getline(input_file, is_local_line);
		this->state_is_local.push_back(stoi(is_local_line));

		string index_line;
		getline(input_file, index_line);
		this->state_indexes.push_back(stoi(index_line));

		string obs_index_line;
		getline(input_file, obs_index_line);
		this->state_obs_indexes.push_back(stoi(obs_index_line));

		string def_id_line;
		getline(input_file, def_id_line);
		this->state_defs.push_back(solution->states[stoi(def_id_line)]);

		string network_index_line;
		getline(input_file, network_index_line);
		this->state_network_indexes.push_back(stoi(network_index_line));
	}

	string next_node_id_line;
	getline(input_file, next_node_id_line);
	this->next_node_id = stoi(next_node_id_line);

	this->experiment = NULL;
}

ScopeNode::~ScopeNode() {
	if (this->experiment != NULL) {
		delete this->experiment;
	}
}

void ScopeNode::save(ofstream& output_file) {
	output_file << this->inner_scope->id << endl;

	output_file << this->starting_node_ids.size() << endl;
	for (int l_index = 0; l_index < (int)this->starting_node_ids.size(); l_index++) {
		output_file << this->starting_node_ids[l_index] << endl;
	}

	output_file << this->input_types.size() << endl;
	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		output_file << this->input_types[i_index] << endl;
		output_file << this->input_inner_layers[i_index] << endl;
		output_file << this->input_inner_is_local[i_index] << endl;
		output_file << this->input_inner_indexes[i_index] << endl;
		output_file << this->input_outer_is_local[i_index] << endl;
		output_file << this->input_outer_indexes[i_index] << endl;
		output_file << this->input_init_vals[i_index] << endl;
	}

	output_file << this->output_inner_indexes.size() << endl;
	for (int o_index = 0; o_index < (int)this->output_inner_indexes.size(); o_index++) {
		output_file << this->output_inner_indexes[o_index] << endl;
		output_file << this->output_outer_is_local[o_index] << endl;
		output_file << this->output_outer_indexes[o_index] << endl;
	}

	output_file << this->state_defs.size() << endl;
	for (int s_index = 0; s_index < (int)this->state_defs.size(); s_index++) {
		output_file << this->state_is_local[s_index] << endl;
		output_file << this->state_indexes[s_index] << endl;
		output_file << this->state_obs_indexes[s_index] << endl;
		output_file << this->state_defs[s_index]->id << endl;
		output_file << this->state_network_indexes[s_index] << endl;
	}

	output_file << this->next_node_id << endl;
}

void ScopeNode::save_for_display(ofstream& output_file) {
	output_file << this->inner_scope->id << endl;

	output_file << this->next_node_id << endl;
}

ScopeNodeHistory::ScopeNodeHistory(ScopeNode* node) {
	this->node = node;

	this->branch_experiment_history = NULL;
}

ScopeNodeHistory::ScopeNodeHistory(ScopeNodeHistory* original) {
	this->node = original->node;

	this->is_halfway = original->is_halfway;
	this->is_early_exit = original->is_early_exit;

	this->inner_scope_history = new ScopeHistory(original->inner_scope_history);

	this->obs_snapshots = original->obs_snapshots;

	if (original->branch_experiment_history != NULL) {
		this->branch_experiment_history = new BranchExperimentHistory(original->branch_experiment_history);
	} else {
		this->branch_experiment_history = NULL;
	}
}

ScopeNodeHistory::~ScopeNodeHistory() {
	delete this->inner_scope_history;

	if (this->branch_experiment_history != NULL) {
		delete this->branch_experiment_history;
	}
}
