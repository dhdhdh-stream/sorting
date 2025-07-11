#include "scope.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "obs_node.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

Scope::Scope() {
	this->new_scope = NULL;

	this->exceeded = false;
	this->generalized = false;
}

Scope::~Scope() {
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		delete this->nodes[n_index];
	}
}

void Scope::clean_inputs(Scope* scope,
						 int node_id) {
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		it->second->clean_inputs(scope,
								 node_id);
	}

	if (this->new_scope != NULL) {
		this->new_scope->clean_inputs(scope,
									  node_id);
	}
}

void Scope::clean_inputs(Scope* scope) {
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		it->second->clean_inputs(scope);
	}

	if (this->new_scope != NULL) {
		this->new_scope->clean_inputs(scope);
	}
}

void Scope::replace_factor(Scope* scope,
						   int original_node_id,
						   int original_factor_index,
						   int new_node_id,
						   int new_factor_index) {
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		it->second->replace_factor(scope,
								   original_node_id,
								   original_factor_index,
								   new_node_id,
								   new_factor_index);
	}

	if (this->new_scope != NULL) {
		this->new_scope->replace_factor(scope,
										original_node_id,
										original_factor_index,
										new_node_id,
										new_factor_index);
	}
}

void Scope::replace_obs_node(Scope* scope,
							 int original_node_id,
							 int new_node_id) {
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		it->second->replace_obs_node(scope,
									 original_node_id,
									 new_node_id);
	}

	if (this->new_scope != NULL) {
		this->new_scope->replace_obs_node(scope,
										  original_node_id,
										  new_node_id);
	}
}

void Scope::replace_scope(Scope* original_scope,
						  Scope* new_scope,
						  int new_scope_node_id) {
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		it->second->replace_scope(original_scope,
								  new_scope,
								  new_scope_node_id);
	}

	if (this->new_scope != NULL) {
		this->new_scope->replace_scope(original_scope,
									   new_scope,
									   new_scope_node_id);

		for (int c_index = 0; c_index < (int)this->new_scope->child_scopes.size(); c_index++) {
			if (this->new_scope->child_scopes[c_index] == original_scope) {
				this->new_scope->child_scopes.push_back(new_scope);
			}
		}
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

	output_file << this->exceeded << endl;
	output_file << this->generalized << endl;
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
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = new ObsNode();
				obs_node->parent = this;
				obs_node->id = id;
				obs_node->load(input_file,
							   parent_solution);
				this->nodes[obs_node->id] = obs_node;
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

	string exceeded_line;
	getline(input_file, exceeded_line);
	this->exceeded = stoi(exceeded_line);

	string generalized_line;
	getline(input_file, generalized_line);
	this->generalized = stoi(generalized_line);
}

void Scope::link(Solution* parent_solution) {
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		it->second->link(parent_solution);
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
}

ScopeHistory::~ScopeHistory() {
	for (map<int, AbstractNodeHistory*>::iterator it = this->node_histories.begin();
			it != this->node_histories.end(); it++) {
		delete it->second;
	}
}
