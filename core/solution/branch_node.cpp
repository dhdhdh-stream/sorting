#include "branch_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "globals.h"
#include "network.h"
#include "scope.h"
#include "solution.h"

using namespace std;

BranchNode::BranchNode() {
	this->type = NODE_TYPE_BRANCH;

	this->network = NULL;

	#if defined(MDEBUG) && MDEBUG
	this->verify_key = NULL;
	#endif /* MDEBUG */
}

BranchNode::BranchNode(BranchNode* original) {
	this->type = NODE_TYPE_BRANCH;

	this->input_scope_context_ids = original->input_scope_context_ids;
	this->input_node_context_ids = original->input_node_context_ids;
	this->input_obs_indexes = original->input_obs_indexes;
	this->network = new Network(original->network);

	this->original_next_node_id = original->original_next_node_id;
	this->branch_next_node_id = original->branch_next_node_id;

	#if defined(MDEBUG) && MDEBUG
	this->verify_key = NULL;
	#endif /* MDEBUG */
}

BranchNode::~BranchNode() {
	delete this->network;

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

void BranchNode::clean_node(int scope_id,
							int node_id) {
	for (int i_index = this->input_node_contexts.size()-1; i_index >= 0; i_index--) {
		bool has_match = false;
		for (int l_index = 0; l_index < (int)this->input_scope_contexts[i_index].size(); l_index++) {
			if (this->input_scope_context_ids[i_index][l_index] == scope_id
					&& this->input_node_context_ids[i_index][l_index] == node_id) {
				has_match = true;
				break;
			}
		}

		if (has_match) {
			this->input_scope_context_ids.erase(this->input_scope_context_ids.begin() + i_index);
			this->input_scope_contexts.erase(this->input_scope_contexts.begin() + i_index);
			this->input_node_context_ids.erase(this->input_node_context_ids.begin() + i_index);
			this->input_node_contexts.erase(this->input_node_contexts.begin() + i_index);
			this->input_obs_indexes.erase(this->input_obs_indexes.begin() + i_index);
			this->network->remove_input(i_index);
		}
	}
}

void BranchNode::save(ofstream& output_file) {
	output_file << this->input_scope_contexts.size() << endl;
	for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
		output_file << this->input_scope_contexts[i_index].size() << endl;
		for (int c_index = 0; c_index < (int)this->input_scope_contexts[i_index].size(); c_index++) {
			output_file << this->input_scope_context_ids[i_index][c_index] << endl;
			output_file << this->input_node_context_ids[i_index][c_index] << endl;
		}
		output_file << this->input_obs_indexes[i_index] << endl;
	}
	this->network->save(output_file);

	output_file << this->original_next_node_id << endl;
	output_file << this->branch_next_node_id << endl;
}

void BranchNode::load(ifstream& input_file) {
	string num_inputs_line;
	getline(input_file, num_inputs_line);
	int num_inputs = stoi(num_inputs_line);
	for (int i_index = 0; i_index < num_inputs; i_index++) {
		string context_size_line;
		getline(input_file, context_size_line);
		int context_size = stoi(context_size_line);
		vector<int> c_scope_context_ids;
		vector<int> c_node_context_ids;
		for (int c_index = 0; c_index < context_size; c_index++) {
			string scope_context_id;
			getline(input_file, scope_context_id);
			c_scope_context_ids.push_back(stoi(scope_context_id));

			string node_context_id;
			getline(input_file, node_context_id);
			c_node_context_ids.push_back(stoi(node_context_id));
		}
		this->input_scope_context_ids.push_back(c_scope_context_ids);
		this->input_node_context_ids.push_back(c_node_context_ids);

		string obs_index_line;
		getline(input_file, obs_index_line);
		this->input_obs_indexes.push_back(stoi(obs_index_line));
	}
	this->network = new Network(input_file);

	string original_next_node_id_line;
	getline(input_file, original_next_node_id_line);
	this->original_next_node_id = stoi(original_next_node_id_line);

	string branch_next_node_id_line;
	getline(input_file, branch_next_node_id_line);
	this->branch_next_node_id = stoi(branch_next_node_id_line);
}

void BranchNode::link(Solution* parent_solution) {
	for (int i_index = 0; i_index < (int)this->input_scope_context_ids.size(); i_index++) {
		vector<AbstractScope*> c_scope_context;
		vector<AbstractNode*> c_node_context;
		for (int c_index = 0; c_index < (int)this->input_scope_context_ids[i_index].size(); c_index++) {
			int scope_id = this->input_scope_context_ids[i_index][c_index];
			Scope* scope = parent_solution->scopes[scope_id];
			c_scope_context.push_back(scope);
			c_node_context.push_back(scope->nodes[this->input_node_context_ids[i_index][c_index]]);
		}
		this->input_scope_contexts.push_back(c_scope_context);
		this->input_node_contexts.push_back(c_node_context);
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

BranchNodeHistory::BranchNodeHistory() {
	// do nothing
}

BranchNodeHistory::BranchNodeHistory(BranchNodeHistory* original) {
	this->index = original->index;

	this->score = original->score;
}
