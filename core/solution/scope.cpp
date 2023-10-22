#include "scope.h"

#include <iostream>

#include "abstract_node.h"
#include "action_node.h"
#include "branch_node.h"
#include "branch_stub_node.h"
#include "exit_node.h"
#include "globals.h"
#include "scale.h"
#include "scope_node.h"
#include "solution.h"
#include "state.h"

using namespace std;

Scope::Scope() {
	// do nothing
}

Scope::~Scope() {
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		delete this->nodes[n_index];
	}

	while (this->scope_histories.size() > 0) {
		delete this->scope_histories.front();
		this->scope_histories.pop_front();
	}
}

void Scope::save(ofstream& output_file) {
	output_file << this->num_input_states << endl;
	for (int s_index = 0; s_index < this->num_input_states; s_index++) {
		output_file << this->input_state_weights[s_index] << endl;
	}

	output_file << this->num_local_states << endl;
	for (int s_index = 0; s_index < this->num_local_states; s_index++) {
		output_file << this->local_state_weights[s_index] << endl;
	}

	output_file << this->nodes.size() << endl;
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		output_file << this->nodes[n_index]->type << endl;
		this->nodes[n_index]->save(output_file);
	}

	output_file << this->average_score << endl;
	output_file << this->score_variance << endl;
	output_file << this->average_misguess << endl;
	output_file << this->misguess_variance << endl;

	output_file << this->child_scopes.size() << endl;
	for (int c_index = 0; c_index < (int)this->child_scopes.size(); c_index++) {
		output_file << this->child_scopes[c_index]->id << endl;
	}
}

void Scope::load(ifstream& input_file,
				 int id) {
	this->id = id;

	string num_input_states_line;
	getline(input_file, num_input_states_line);
	this->num_input_states = stoi(num_input_states_line);
	for (int s_index = 0; s_index < this->num_input_states; s_index++) {
		string weight_line;
		getline(input_file, weight_line);
		this->input_state_weights.push_back(stod(weight_line));
	}

	string num_local_states_line;
	getline(input_file, num_local_states_line);
	this->num_local_states = stoi(num_local_states_line);
	for (int s_index = 0; s_index < this->num_local_states; s_index++) {
		string weight_line;
		getline(input_file, weight_line);
		this->local_state_weights.push_back(stod(weight_line));
	}

	string num_nodes_line;
	getline(input_file, num_nodes_line);
	int num_nodes = stoi(num_nodes_line);
	for (int n_index = 0; n_index < num_nodes; n_index++) {
		string type_line;
		getline(input_file, type_line);
		int type = stoi(type_line);
		if (type == NODE_TYPE_ACTION) {
			ActionNode* node = new ActionNode(input_file,
											  n_index);
			this->nodes.push_back(node);
		} else if (type == NODE_TYPE_SCOPE) {
			ScopeNode* node = new ScopeNode(input_file,
											n_index);
			this->nodes.push_back(node);
		} else if (type == NODE_TYPE_BRANCH) {
			BranchNode* node = new BranchNode(input_file,
											  n_index);
			this->nodes.push_back(node);
		} else if (type == NODE_TYPE_BRANCH_STUB) {
			BranchStubNode* node = new BranchStubNode(input_file,
													  n_index);
			this->nodes.push_back(node);
		} else {
			ExitNode* node = new ExitNode(input_file,
										  n_index);
			this->nodes.push_back(node);
		}
	}

	string average_score_line;
	getline(input_file, average_score_line);
	this->average_score = stod(average_score_line);

	string score_variance_line;
	getline(input_file, score_variance_line);
	this->score_variance = stod(score_variance_line);

	string average_misguess_line;
	getline(input_file, average_misguess_line);
	this->average_misguess = stod(average_misguess_line);

	string misguess_variance_line;
	getline(input_file, misguess_variance_line);
	this->misguess_variance = stod(misguess_variance_line);

	string child_scopes_size_line;
	getline(input_file, child_scopes_size_line);
	int child_scopes_size = stoi(child_scopes_size_line);
	for (int c_index = 0; c_index < child_scopes_size; c_index++) {
		string scope_id_line;
		getline(input_file, scope_id_line);
		this->child_scopes.push_back(solution->scopes[stoi(scope_id_line)]);
	}
}

void Scope::save_for_display(ofstream& output_file) {
	output_file << this->nodes.size() << endl;
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		output_file << this->nodes[n_index]->type << endl;
		this->nodes[n_index]->save_for_display(output_file);
	}
}

ScopeHistory::ScopeHistory(Scope* scope) {
	this->scope = scope;

	this->inner_branch_experiment_history = NULL;

	this->exceeded_depth = false;
}

ScopeHistory::ScopeHistory(ScopeHistory* original) {
	this->scope = original->scope;

	for (int i_index = 0; i_index < (int)original->node_histories.size(); i_index++) {
		this->node_histories.push_back(vector<AbstractNodeHistory*>());
		for (int h_index = 0; h_index < (int)original->node_histories[i_index].size(); h_index++) {
			if (original->node_histories[i_index][h_index]->node->type == NODE_TYPE_ACTION) {
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)original->node_histories[i_index][h_index];
				this->node_histories.back().push_back(new ActionNodeHistory(action_node_history));
			} else if (original->node_histories[i_index][h_index]->node->type == NODE_TYPE_SCOPE) {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)original->node_histories[i_index][h_index];
				this->node_histories.back().push_back(new ScopeNodeHistory(scope_node_history));
			} else if (original->node_histories[i_index][h_index]->node->type == NODE_TYPE_BRANCH) {
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)original->node_histories[i_index][h_index];
				this->node_histories.back().push_back(new BranchNodeHistory(branch_node_history));
			}
		}
	}

	this->inner_branch_experiment_history = NULL;

	this->exceeded_depth = original->exceeded_depth;
}

ScopeHistory::~ScopeHistory() {
	for (int i_index = 0; i_index < (int)this->node_histories.size(); i_index++) {
		for (int h_index = 0; h_index < (int)this->node_histories[i_index].size(); h_index++) {
			delete this->node_histories[i_index][h_index];
		}
	}
}
