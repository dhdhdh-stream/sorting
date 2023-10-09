#include "branch_node.h"

#include "branch_experiment.h"
#include "globals.h"
#include "solution.h"
#include "state.h"

using namespace std;

BranchNode::BranchNode() {
	this->type = NODE_TYPE_BRANCH;

	this->experiment = NULL;
}

BranchNode::BranchNode(ifstream& input_file,
					   int id) {
	this->type = NODE_TYPE_BRANCH;

	this->id = id;

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

	string shared_state_is_local_size_line;
	getline(input_file, shared_state_is_local_size_line);
	int shared_state_is_local_size = stoi(shared_state_is_local_size_line);
	for (int s_index = 0; s_index < shared_state_is_local_size; s_index++) {
		string is_local_line;
		getline(input_file, is_local_line);
		this->shared_state_is_local.push_back(stoi(is_local_line));

		string index_line;
		getline(input_file, index_line);
		this->shared_state_indexes.push_back(stoi(index_line));

		string branch_weight_line;
		getline(input_file, branch_weight_line);
		this->branch_weights.push_back(stod(branch_weight_line));

		string def_id_line;
		getline(input_file, def_id_line);
		this->original_state_defs.push_back(solution->states[stoi(def_id_line)]);
	}

	string branch_state_is_local_size_line;
	getline(input_file, branch_state_is_local_size_line);
	int branch_state_is_local_size = stoi(branch_state_is_local_size_line);
	for (int s_index = 0; s_index < branch_state_is_local_size; s_index++) {
		string is_local_line;
		getline(input_file, is_local_line);
		this->branch_state_is_local.push_back(stoi(is_local_line));

		string index_line;
		getline(input_file, index_line);
		this->branch_state_indexes.push_back(stoi(index_line));

		string def_id_line;
		getline(input_file, def_id_line);
		this->branch_state_defs.push_back(solution->states[stoi(def_id_line)]);
	}

	string branch_next_node_id_line;
	getline(input_file, branch_next_node_id_line);
	this->branch_next_node_id = stoi(branch_next_node_id_line);

	string original_next_node_id_line;
	getline(input_file, original_next_node_id_line);
	this->original_next_node_id = stoi(original_next_node_id_line);

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

	this->experiment = NULL;
}

BranchNode::~BranchNode() {
	if (this->experiment != NULL) {
		delete this->experiment;
	}
}

void BranchNode::save(ofstream& output_file) {
	output_file << this->branch_scope_context.size() << endl;
	for (int c_index = 0; c_index < (int)this->branch_scope_context.size(); c_index++) {
		output_file << this->branch_scope_context[c_index] << endl;
		output_file << this->branch_node_context[c_index] << endl;
	}

	output_file << this->branch_is_pass_through << endl;

	output_file << this->shared_state_is_local.size() << endl;
	for (int s_index = 0; s_index < (int)this->shared_state_is_local.size(); s_index++) {
		output_file << this->shared_state_is_local[s_index] << endl;
		output_file << this->shared_state_indexes[s_index] << endl;
		output_file << this->branch_weights[s_index] << endl;
		output_file << this->original_state_defs[s_index]->id << endl;
	}

	output_file << this->branch_state_is_local.size() << endl;
	for (int s_index = 0; s_index < (int)this->branch_state_is_local.size(); s_index++) {
		output_file << this->branch_state_is_local[s_index] << endl;
		output_file << this->branch_state_indexes[s_index] << endl;
		output_file << this->branch_state_defs[s_index]->id << endl;
	}

	output_file << this->branch_next_node_id << endl;
	output_file << this->original_next_node_id << endl;

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
}

BranchNodeHistory::BranchNodeHistory(BranchNode* node) {
	this->node = node;

	this->branch_experiment_history = NULL;
}

BranchNodeHistory::~BranchNodeHistory() {
	if (this->branch_experiment_history != NULL) {
		delete this->branch_experiment_history;
	}
}