#include "scope.h"

#include <iostream>

#include "abstract_experiment.h"
#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "network.h"
#include "return_node.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

Scope::Scope() {
	this->id = -1;

	this->new_action_experiment = NULL;
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

void Scope::clean() {
	map<int, AbstractNode*>::iterator it = this->nodes.begin();
	while (it != this->nodes.end()) {
		switch (it->second->type) {
		case NODE_TYPE_RETURN:
			{
				ReturnNode* return_node = (ReturnNode*)it->second;
				if (return_node->previous_location_id == -1) {
					clean_scope_node_helper(this,
											return_node,
											return_node->skipped_next_node);

					delete it->second;
					it = this->nodes.erase(it);

					continue;
				}
			}
			break;
		}

		it++;
	}

	clean_branch_node(this);
}

void Scope::clean_node(int node_id) {
	map<int, AbstractNode*>::iterator it = this->nodes.begin();
	while (it != this->nodes.end()) {
		switch (it->second->type) {
		case NODE_TYPE_RETURN:
			{
				ReturnNode* return_node = (ReturnNode*)it->second;
				if (return_node->previous_location_id == node_id) {
					clean_scope_node_helper(this,
											return_node,
											return_node->skipped_next_node);

					delete it->second;
					it = this->nodes.erase(it);

					continue;
				}
			}
			break;
		}

		it++;
	}
}

void Scope::save(ofstream& output_file) {
	output_file << this->node_counter << endl;

	output_file << this->nodes.size() << endl;
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		output_file << it->first << endl;
		output_file << it->second->type << endl;
		it->second->save(output_file);
	}

	output_file << this->child_scopes.size() << endl;
	for (int c_index = 0; c_index < (int)this->child_scopes.size(); c_index++) {
		output_file << this->child_scopes[c_index]->id << endl;
	}
}

void Scope::load(ifstream& input_file,
				 Solution* parent_solution) {
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
		switch (type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = new ActionNode();
				action_node->parent = this;
				action_node->id = id;
				action_node->load(input_file);
				this->nodes[action_node->id] = action_node;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = new ScopeNode();
				scope_node->parent = this;
				scope_node->id = id;
				scope_node->load(input_file,
								 parent_solution);
				this->nodes[scope_node->id] = scope_node;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = new BranchNode();
				branch_node->parent = this;
				branch_node->id = id;
				branch_node->load(input_file);
				this->nodes[branch_node->id] = branch_node;
			}
			break;
		case NODE_TYPE_RETURN:
			{
				ReturnNode* return_node = new ReturnNode();
				return_node->parent = this;
				return_node->id = id;
				return_node->load(input_file);
				this->nodes[return_node->id] = return_node;
			}
			break;
		}
	}

	string num_child_scopes_line;
	getline(input_file, num_child_scopes_line);
	int num_child_scopes = stoi(num_child_scopes_line);
	for (int c_index = 0; c_index < num_child_scopes; c_index++) {
		string scope_id_line;
		getline(input_file, scope_id_line);
		this->child_scopes.push_back(parent_solution->scopes[stoi(scope_id_line)]);
	}
}

void Scope::link(Solution* parent_solution) {
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		it->second->link(parent_solution);
	}
}

void Scope::copy_from(Scope* original,
					  Solution* parent_solution) {
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
		case NODE_TYPE_RETURN:
			{
				ReturnNode* original_return_node = (ReturnNode*)it->second;
				ReturnNode* new_return_node = new ReturnNode(original_return_node);
				new_return_node->parent = this;
				new_return_node->id = it->first;
				this->nodes[it->first] = new_return_node;
			}
			break;
		}
	}

	for (int c_index = 0; c_index < (int)original->child_scopes.size(); c_index++) {
		this->child_scopes.push_back(parent_solution->scopes[
			original->child_scopes[c_index]->id]);
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
