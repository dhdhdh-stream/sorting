#include "scope.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "exit_node.h"
#include "scope_node.h"

using namespace std;

Scope::Scope() {
	this->id = -1;
}

Scope::~Scope() {
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		delete this->nodes[n_index];
	}
}

#if defined(MDEBUG) && MDEBUG
void Scope::clear_verify() {
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		if (it->second->type == NODE_TYPE_BRANCH) {
			BranchNode* branch_node = (BranchNode*)it->second;
			branch_node->clear_verify();
		}
	}
}
#endif /* MDEBUG */

void Scope::success_reset() {
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		switch (it->second->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)it->second;
				action_node->success_reset();
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)it->second;
				scope_node->success_reset();
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)it->second;
				branch_node->success_reset();
			}
			break;
		}
	}
}

void Scope::fail_reset() {
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		switch (it->second->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)it->second;
				action_node->fail_reset();
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)it->second;
				scope_node->fail_reset();
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)it->second;
				branch_node->fail_reset();
			}
			break;
		}
	}
}

void Scope::save(ofstream& output_file) {
	output_file << this->name << endl;

	output_file << this->node_counter << endl;

	output_file << this->nodes.size() << endl;
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		output_file << it->first << endl;
		output_file << it->second->type << endl;
		it->second->save(output_file);
	}

	output_file << this->default_starting_node_id << endl;
}

void Scope::load(ifstream& input_file) {
	getline(input_file, this->name);

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

	string default_starting_node_id_line;
	getline(input_file, default_starting_node_id_line);
	this->default_starting_node_id = stoi(default_starting_node_id_line);
}

void Scope::link() {
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		it->second->link();
	}

	this->default_starting_node = this->nodes[this->default_starting_node_id];
}

void Scope::remap() {
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		if (it->second->type == NODE_TYPE_BRANCH) {
			BranchNode* branch_node = (BranchNode*)it->second;
			branch_node->remap();
		}
	}
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

	this->pass_through_experiment_history = NULL;
}

ScopeHistory::ScopeHistory(ScopeHistory* original) {
	this->scope = original->scope;

	for (int h_index = 0; h_index < (int)original->node_histories.size(); h_index++) {
		AbstractNodeHistory* node_history = original->node_histories[h_index];
		switch (node_history->node->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)node_history;
				this->node_histories.push_back(new ActionNodeHistory(action_node_history));
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)node_history;
				this->node_histories.push_back(new ScopeNodeHistory(scope_node_history));
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)node_history;
				this->node_histories.push_back(new BranchNodeHistory(branch_node_history));
			}
			break;
		}
	}

	this->pass_through_experiment_history = NULL;
}

ScopeHistory::~ScopeHistory() {
	for (int h_index = 0; h_index < (int)this->node_histories.size(); h_index++) {
		delete this->node_histories[h_index];
	}
}
