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

	this->original_network = NULL;
	this->branch_network = NULL;

	#if defined(MDEBUG) && MDEBUG
	this->verify_key = NULL;
	#endif /* MDEBUG */
}

BranchNode::BranchNode(BranchNode* original) {
	this->type = NODE_TYPE_BRANCH;

	this->original_average_score = original->original_average_score;
	this->branch_average_score = original->branch_average_score;

	this->input_scope_context_ids = original->input_scope_context_ids;
	this->input_node_context_ids = original->input_node_context_ids;
	this->input_obs_indexes = original->input_obs_indexes;

	this->linear_original_input_indexes = original->linear_original_input_indexes;
	this->linear_original_weights = original->linear_original_weights;

	this->linear_branch_input_indexes = original->linear_branch_input_indexes;
	this->linear_branch_weights = original->linear_branch_weights;

	this->original_network_input_indexes = original->original_network_input_indexes;
	if (original->original_network == NULL) {
		this->original_network = NULL;
	} else {
		this->original_network = new Network(original->original_network);
	}

	this->branch_network_input_indexes = original->branch_network_input_indexes;
	if (original->branch_network == NULL) {
		this->branch_network = NULL;
	} else {
		this->branch_network = new Network(original->branch_network);
	}

	this->original_next_node_id = original->original_next_node_id;
	this->branch_next_node_id = original->branch_next_node_id;

	#if defined(MDEBUG) && MDEBUG
	this->verify_key = NULL;
	#endif /* MDEBUG */
}

BranchNode::~BranchNode() {
	if (this->original_network != NULL) {
		delete this->original_network;
	}

	if (this->branch_network != NULL) {
		delete this->branch_network;
	}

	for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
		delete this->experiments[e_index];
	}
}

#if defined(MDEBUG) && MDEBUG
void BranchNode::clear_verify() {
	this->verify_key = NULL;
	if (this->verify_original_scores.size() > 0
			|| this->verify_branch_scores.size() > 0) {
		cout << "seed: " << seed << endl;

		throw invalid_argument("branch node remaining verify");
	}
}
#endif /* MDEBUG */

void BranchNode::save(ofstream& output_file) {
	output_file << this->original_average_score << endl;
	output_file << this->branch_average_score << endl;

	output_file << this->input_scope_contexts.size() << endl;
	for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
		output_file << this->input_scope_contexts[i_index].size() << endl;
		for (int c_index = 0; c_index < (int)this->input_scope_contexts[i_index].size(); c_index++) {
			output_file << this->input_scope_context_ids[i_index][c_index] << endl;
			output_file << this->input_node_context_ids[i_index][c_index] << endl;
		}
		output_file << this->input_obs_indexes[i_index] << endl;
	}

	output_file << this->linear_original_input_indexes.size() << endl;
	for (int i_index = 0; i_index < (int)this->linear_original_input_indexes.size(); i_index++) {
		output_file << this->linear_original_input_indexes[i_index] << endl;
		output_file << this->linear_original_weights[i_index] << endl;
	}

	output_file << this->linear_branch_input_indexes.size() << endl;
	for (int i_index = 0; i_index < (int)this->linear_branch_input_indexes.size(); i_index++) {
		output_file << this->linear_branch_input_indexes[i_index] << endl;
		output_file << this->linear_branch_weights[i_index] << endl;
	}

	output_file << this->original_network_input_indexes.size() << endl;
	for (int i_index = 0; i_index < (int)this->original_network_input_indexes.size(); i_index++) {
		output_file << this->original_network_input_indexes[i_index].size() << endl;
		for (int v_index = 0; v_index < (int)this->original_network_input_indexes[i_index].size(); v_index++) {
			output_file << this->original_network_input_indexes[i_index][v_index] << endl;
		}
	}
	if (this->original_network != NULL) {
		this->original_network->save(output_file);
	}

	output_file << this->branch_network_input_indexes.size() << endl;
	for (int i_index = 0; i_index < (int)this->branch_network_input_indexes.size(); i_index++) {
		output_file << this->branch_network_input_indexes[i_index].size() << endl;
		for (int v_index = 0; v_index < (int)this->branch_network_input_indexes[i_index].size(); v_index++) {
			output_file << this->branch_network_input_indexes[i_index][v_index] << endl;
		}
	}
	if (this->branch_network != NULL) {
		this->branch_network->save(output_file);
	}

	output_file << this->original_next_node_id << endl;
	output_file << this->branch_next_node_id << endl;
}

void BranchNode::load(ifstream& input_file) {
	string original_average_score_line;
	getline(input_file, original_average_score_line);
	this->original_average_score = stod(original_average_score_line);

	string branch_average_score_line;
	getline(input_file, branch_average_score_line);
	this->branch_average_score = stod(branch_average_score_line);

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

	string num_linear_original_inputs_line;
	getline(input_file, num_linear_original_inputs_line);
	int num_linear_original_inputs = stoi(num_linear_original_inputs_line);
	for (int i_index = 0; i_index < num_linear_original_inputs; i_index++) {
		string input_index_line;
		getline(input_file, input_index_line);
		this->linear_original_input_indexes.push_back(stoi(input_index_line));

		string weight_line;
		getline(input_file, weight_line);
		this->linear_original_weights.push_back(stod(weight_line));
	}

	string num_linear_branch_inputs_line;
	getline(input_file, num_linear_branch_inputs_line);
	int num_linear_branch_inputs = stoi(num_linear_branch_inputs_line);
	for (int i_index = 0; i_index < num_linear_branch_inputs; i_index++) {
		string input_index_line;
		getline(input_file, input_index_line);
		this->linear_branch_input_indexes.push_back(stoi(input_index_line));

		string weight_line;
		getline(input_file, weight_line);
		this->linear_branch_weights.push_back(stod(weight_line));
	}

	string num_original_network_inputs_line;
	getline(input_file, num_original_network_inputs_line);
	int num_original_network_inputs = stoi(num_original_network_inputs_line);
	for (int i_index = 0; i_index < num_original_network_inputs; i_index++) {
		string layer_size_line;
		getline(input_file, layer_size_line);
		int layer_size = stoi(layer_size_line);
		vector<int> v_input_indexes;
		for (int v_index = 0; v_index < layer_size; v_index++) {
			string input_index_line;
			getline(input_file, input_index_line);
			v_input_indexes.push_back(stoi(input_index_line));
		}
		this->original_network_input_indexes.push_back(v_input_indexes);
	}
	if (this->original_network_input_indexes.size() == 0) {
		this->original_network = NULL;
	} else {
		this->original_network = new Network(input_file);
	}

	string num_branch_network_inputs_line;
	getline(input_file, num_branch_network_inputs_line);
	int num_branch_network_inputs = stoi(num_branch_network_inputs_line);
	for (int i_index = 0; i_index < num_branch_network_inputs; i_index++) {
		string layer_size_line;
		getline(input_file, layer_size_line);
		int layer_size = stoi(layer_size_line);
		vector<int> v_input_indexes;
		for (int v_index = 0; v_index < layer_size; v_index++) {
			string input_index_line;
			getline(input_file, input_index_line);
			v_input_indexes.push_back(stoi(input_index_line));
		}
		this->branch_network_input_indexes.push_back(v_input_indexes);
	}
	if (this->branch_network_input_indexes.size() == 0) {
		this->branch_network = NULL;
	} else {
		this->branch_network = new Network(input_file);
	}

	string original_next_node_id_line;
	getline(input_file, original_next_node_id_line);
	this->original_next_node_id = stoi(original_next_node_id_line);

	string branch_next_node_id_line;
	getline(input_file, branch_next_node_id_line);
	this->branch_next_node_id = stoi(branch_next_node_id_line);
}

void BranchNode::link(Solution* parent_solution) {
	for (int i_index = 0; i_index < (int)this->input_scope_context_ids.size(); i_index++) {
		vector<Scope*> c_scope_context;
		vector<AbstractNode*> c_node_context;
		for (int c_index = 0; c_index < (int)this->input_scope_context_ids[i_index].size(); c_index++) {
			Scope* scope;
			int scope_id = this->input_scope_context_ids[i_index][c_index];
			if (scope_id == -1) {
				scope = parent_solution->current;
			} else {
				scope = parent_solution->scopes[scope_id];
			}
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

void BranchNode::link() {
	for (int i_index = 0; i_index < (int)this->input_scope_context_ids.size(); i_index++) {
		this->input_scope_contexts.push_back(vector<Scope*>{this->parent});
		this->input_node_contexts.push_back(vector<AbstractNode*>{
			this->parent->nodes[this->input_node_context_ids[i_index][0]]});
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

	this->is_branch = original->is_branch;
}
