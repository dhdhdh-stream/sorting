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

void Scope::save(ofstream& output_file) {
	output_file << this->parent_id << endl;
	output_file << this->child_ids.size() << endl;
	for (int c_index = 0; c_index < (int)this->child_ids.size(); c_index++) {
		output_file << this->child_ids[c_index] << endl;
	}
	output_file << this->layer << endl;
	output_file << this->num_improvements << endl;

	output_file << this->node_counter << endl;

	output_file << this->nodes.size() << endl;
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		output_file << it->first << endl;
		output_file << it->second->type << endl;
		it->second->save(output_file);
	}

	output_file << this->default_starting_node_id << endl;

	output_file << this->subscopes.size() << endl;
	for (set<pair<int,set<int>>>::iterator it = this->subscopes.begin();
			it != this->subscopes.end(); it++) {
		output_file << it->first << endl;
		output_file << it->second.size() << endl;
		for (set<int>::iterator exit_it = it->second.begin();
				exit_it != it->second.end(); exit_it++)  {
			output_file << *exit_it << endl;
		}
	}
}

void Scope::load(ifstream& input_file,
				 Solution* parent_solution) {
	string parent_id_line;
	getline(input_file, parent_id_line);
	this->parent_id = stoi(parent_id_line);

	string num_child_ids_line;
	getline(input_file, num_child_ids_line);
	int num_child_ids = stoi(num_child_ids_line);
	for (int c_index = 0; c_index < num_child_ids; c_index++) {
		string child_id_line;
		getline(input_file, child_id_line);
		this->child_ids.push_back(stoi(child_id_line));
	}

	string layer_line;
	getline(input_file, layer_line);
	this->layer = stoi(layer_line);

	string num_improvements_line;
	getline(input_file, num_improvements_line);
	this->num_improvements = stoi(num_improvements_line);

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
			scope_node->load(input_file,
							 parent_solution);
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
			exit_node->load(input_file,
							parent_solution);
			this->nodes[exit_node->id] = exit_node;
		}
	}

	string default_starting_node_id_line;
	getline(input_file, default_starting_node_id_line);
	this->default_starting_node_id = stoi(default_starting_node_id_line);

	string subscopes_size_line;
	getline(input_file, subscopes_size_line);
	int subscope_size = stoi(subscopes_size_line);
	for (int s_index = 0; s_index < subscope_size; s_index++) {
		string start_id_line;
		getline(input_file, start_id_line);

		string num_exit_ids_line;
		getline(input_file, num_exit_ids_line);
		int num_exit_ids = stoi(num_exit_ids_line);
		set<int> exit_ids;
		for (int e_index = 0; e_index < num_exit_ids; e_index++) {
			string exit_id_line;
			getline(input_file, exit_id_line);
			exit_ids.insert(stoi(exit_id_line));
		}

		this->subscopes.insert({stoi(start_id_line), exit_ids});
	}
}

void Scope::link(Solution* parent_solution) {
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		it->second->link(parent_solution);
	}

	this->default_starting_node = this->nodes[this->default_starting_node_id];
}

void Scope::copy_from(Scope* original,
					  Solution* parent_solution) {
	this->parent_id = original->parent_id;
	this->child_ids = original->child_ids;
	this->layer = original->layer;
	this->num_improvements = original->num_improvements;

	this->node_counter = original->node_counter;

	for (map<int, AbstractNode*>::iterator it = original->nodes.begin();
			it != original->nodes.end(); it++) {
		switch (it->second->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* original_action_node = (ActionNode*)it->second;
				ActionNode* new_action_node = new ActionNode(original_action_node);
				new_action_node->parent = this;
				new_action_node->id = it->first;
				this->nodes[it->first] = new_action_node;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* original_scope_node = (ScopeNode*)it->second;
				ScopeNode* new_scope_node = new ScopeNode(original_scope_node,
														  parent_solution);
				new_scope_node->parent = this;
				new_scope_node->id = it->first;
				this->nodes[it->first] = new_scope_node;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* original_branch_node = (BranchNode*)it->second;
				BranchNode* new_branch_node = new BranchNode(original_branch_node);
				new_branch_node->parent = this;
				new_branch_node->id = it->first;
				this->nodes[it->first] = new_branch_node;
			}
			break;
		case NODE_TYPE_EXIT:
			{
				ExitNode* original_exit_node = (ExitNode*)it->second;
				ExitNode* new_exit_node = new ExitNode(original_exit_node,
													   parent_solution);
				new_exit_node->parent = this;
				new_exit_node->id = it->first;
				this->nodes[it->first] = new_exit_node;
			}
			break;
		}
	}

	this->default_starting_node_id = original->default_starting_node_id;

	this->subscopes = original->subscopes;
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

	this->experiment_history = NULL;
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

	this->experiment_history = NULL;
}

ScopeHistory::~ScopeHistory() {
	for (int h_index = 0; h_index < (int)this->node_histories.size(); h_index++) {
		delete this->node_histories[h_index];
	}
}
