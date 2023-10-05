#include "action_node.h"

#include <iostream>

#include "branch_experiment.h"
#include "globals.h"
#include "solution.h"
#include "state.h"

using namespace std;

ActionNode::ActionNode() {
	this->type = NODE_TYPE_ACTION;

	this->experiment = NULL;
}

ActionNode::ActionNode(ifstream& input_file,
					   int id) {
	this->type = NODE_TYPE_ACTION;

	this->id = id;

	string state_defs_size_line;
	getline(input_file, state_defs_size_line);
	int state_defs_size = stoi(state_defs_size_line);
	for (int s_index = 0; s_index < state_defs_size; s_index++) {
		string is_local_line;
		getline(input_file, is_local_line);
		this->state_is_local.push_back(stoi(is_local_line));

		string indexes_line;
		getline(input_file, indexes_line);
		this->state_indexes.push_back(stoi(indexes_line));

		string def_id_line;
		getline(input_file, def_id_line);
		this->state_defs.push_back(solution->states[stoi(def_id_line)]);

		string network_index_line;
		getline(input_file, network_index_line);
		this->state_network_indexes.push_back(stoi(network_index_line));
	}

	string score_state_defs_size_line;
	getline(input_file, score_state_defs_size_line);
	int score_state_defs_size = stoi(score_state_defs_size_line);
	for (int s_index = 0; s_index < score_state_defs_size; s_index++) {
		string context_size_line;
		getline(input_file, context_size_line);
		int context_size = stoi(context_size_line);
		this->score_state_scope_contexts.push_back(vector<int>());
		this->score_state_node_contexts.push_back(vector<int>());
		for (int c_index = 0; c_index < context_size; c_index++) {
			string scope_context_line;
			getline(input_file, scope_context_line);
			this->score_state_scope_contexts.back().push_back(stoi(scope_context_line));

			string node_context_line;
			getline(input_file, node_context_line);
			this->score_state_node_contexts.back().push_back(stoi(node_context_line));
		}

		string def_id_line;
		getline(input_file, def_id_line);
		this->score_state_defs.push_back(solution->states[stoi(def_id_line)]);

		string network_index_line;
		getline(input_file, network_index_line);
		this->score_state_network_indexes.push_back(stoi(network_index_line));

		this->score_state_defs.back()->nodes[this->score_state_network_indexes.back()] = this;
	}

	string next_node_id_line;
	getline(input_file, next_node_id_line);
	this->next_node_id = stoi(next_node_id_line);

	this->experiment = NULL;
}

ActionNode::~ActionNode() {
	if (this->experiment != NULL) {
		delete this->experiment;
	}
}

void ActionNode::save(ofstream& output_file) {
	output_file << this->state_defs.size() << endl;
	for (int s_index = 0; s_index < (int)this->state_defs.size(); s_index++) {
		output_file << this->state_is_local[s_index] << endl;
		output_file << this->state_indexes[s_index] << endl;
		output_file << this->state_defs[s_index]->id << endl;
		output_file << this->state_network_indexes[s_index] << endl;
	}

	output_file << this->score_state_defs.size() << endl;
	for (int s_index = 0; s_index < (int)this->score_state_defs.size(); s_index++) {
		output_file << this->score_state_scope_contexts[s_index].size() << endl;
		for (int c_index = 0; c_index < (int)this->score_state_scope_contexts[s_index].size(); c_index++) {
			output_file << this->score_state_scope_contexts[s_index][c_index] << endl;
			output_file << this->score_state_node_contexts[s_index][c_index] << endl;
		}
		output_file << this->score_state_defs[s_index]->id << endl;
		output_file << this->score_state_network_indexes[s_index] << endl;
	}

	output_file << this->next_node_id << endl;
}

ActionNodeHistory::ActionNodeHistory(ActionNode* node) {
	this->node = node;

	this->branch_experiment_history = NULL;
}

ActionNodeHistory::~ActionNodeHistory() {
	if (this->branch_experiment_history != NULL) {
		delete this->branch_experiment_history;
	}
}
