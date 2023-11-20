#include "scope.h"

#include <iostream>

#include "abstract_node.h"
#include "action_node.h"
#include "branch_node.h"
#include "exit_node.h"
#include "globals.h"
#include "scope_node.h"
#include "solution.h"
#include "state.h"

using namespace std;

Scope::Scope() {
	this->id = -1;
}

Scope::~Scope() {
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		delete this->nodes[n_index];
	}

	for (int s_index = 0; s_index < (int)this->temp_states.size(); s_index++) {
		delete this->temp_states[s_index];
	}
}

void Scope::success_reset() {
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		if (it->second->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)it->second;
			action_node->success_reset();
		} else if (it->second->type == NODE_TYPE_SCOPE) {
			ScopeNode* scope_node = (ScopeNode*)it->second;
			scope_node->success_reset();
		}
	}

	for (int s_index = 0; s_index < (int)this->temp_states.size(); s_index++) {
		if (this->temp_state_new_local_indexes[s_index] == -1) {
			delete this->temp_states[s_index];
		}
	}
	this->temp_states.clear();
	this->temp_state_nodes.clear();
	this->temp_state_scope_contexts.clear();
	this->temp_state_node_contexts.clear();
	this->temp_state_obs_indexes.clear();
	this->temp_state_new_local_indexes.clear();
}

void Scope::fail_reset() {
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		if (it->second->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)it->second;
			action_node->fail_reset();
		} else if (it->second->type == NODE_TYPE_SCOPE) {
			ScopeNode* scope_node = (ScopeNode*)it->second;
			scope_node->fail_reset();
		}
	}
}

void Scope::save(ofstream& output_file) {
	output_file << this->num_input_states << endl;
	output_file << this->num_local_states << endl;

	output_file << this->node_counter << endl;

	output_file << this->nodes.size() << endl;
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		output_file << it->first << endl;
		output_file << it->second->type << endl;
		it->second->save(output_file);
	}

	output_file << this->starting_node_id << endl;
}

void Scope::load(ifstream& input_file) {
	string num_input_states_line;
	getline(input_file, num_input_states_line);
	this->num_input_states = stoi(num_input_states_line);

	string num_local_states_line;
	getline(input_file, num_local_states_line);
	this->num_local_states = stoi(num_local_states_line);

	string node_counter_line;
	getline(input_file, node_counter_line);
	this->node_counter = stoi(node_counter_line);

	string num_nodes_line;
	getline(input_file, num_nodes_line);
	int num_nodes = stoi(num_nodes_line);
	for (int n_index = 0; n_index < num_nodes; n_index++) {
		string id_line;
		getline(input_file, id_line);
		int id = stoi(id_line);

		string type_line;
		getline(input_file, type_line);
		int type = stoi(type_line);
		if (type == NODE_TYPE_ACTION) {
			ActionNode* action_node = new ActionNode();
			action_node->parent = this;
			action_node->id = id;
			action_node->load(input_file);
			this->nodes[action_node->id] = action_node;
		} else if (type == NODE_TYPE_SCOPE) {
			ScopeNode* scope_node = new ScopeNode();
			scope_node->parent = this;
			scope_node->id = id;
			scope_node->load(input_file);
			this->nodes[scope_node->id] = scope_node;
		} else if (type == NODE_TYPE_BRANCH) {
			BranchNode* branch_node = new BranchNode();
			branch_node->parent = this;
			branch_node->id = id;
			branch_node->load(input_file);
			this->nodes[branch_node->id] = branch_node;
		} else {
			ExitNode* exit_node = new ExitNode();
			exit_node->parent = this;
			exit_node->id = id;
			exit_node->load(input_file);
			this->nodes[exit_node->id] = exit_node;
		}
	}

	string starting_node_id_line;
	getline(input_file, starting_node_id_line);
	this->starting_node_id = stoi(starting_node_id_line);
}

void Scope::link() {
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		it->second->link();
	}

	this->starting_node = this->nodes[this->starting_node_id];
}

void Scope::save_for_display(ofstream& output_file) {
	output_file << this->nodes.size() << endl;
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		output_file << it->first << endl;
		output_file << it->second->type << endl;
		it->second->save_for_display(output_file);
	}
}

ScopeHistory::ScopeHistory(Scope* scope) {
	this->scope = scope;

	this->inner_pass_through_experiment = NULL;

	this->experiment_iter_index = -1;
	this->experiment_index = -1;

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
			} else {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)original->node_histories[i_index][h_index];
				this->node_histories.back().push_back(new ScopeNodeHistory(scope_node_history));
			}
		}
	}

	this->inner_pass_through_experiment = NULL;

	this->experiment_iter_index = original->experiment_iter_index;
	this->experiment_index = original->experiment_index;

	this->exceeded_depth = original->exceeded_depth;
}

ScopeHistory::~ScopeHistory() {
	for (int i_index = 0; i_index < (int)this->node_histories.size(); i_index++) {
		for (int h_index = 0; h_index < (int)this->node_histories[i_index].size(); h_index++) {
			delete this->node_histories[i_index][h_index];
		}
	}
}
