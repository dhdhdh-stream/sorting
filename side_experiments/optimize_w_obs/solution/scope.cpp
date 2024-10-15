#include "scope.h"

#include <iostream>

#include "abstract_experiment.h"
#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "network.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

Scope::Scope() {
	this->new_scope_experiment = NULL;
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

void Scope::clean_unneeded_branch_nodes() {
	while (true) {
		bool removed_node = false;

		map<int, AbstractNode*>::iterator it = this->nodes.begin();
		while (it != this->nodes.end()) {
			if (it->second->type == NODE_TYPE_BRANCH) {
				BranchNode* branch_node = (BranchNode*)it->second;
				if (branch_node->original_next_node == branch_node->branch_next_node) {
					for (map<int, AbstractNode*>::iterator inner_it = this->nodes.begin();
							inner_it != this->nodes.end(); inner_it++) {
						switch (inner_it->second->type) {
						case NODE_TYPE_ACTION:
							{
								ActionNode* node = (ActionNode*)inner_it->second;
								if (node->next_node == branch_node) {
									node->next_node_id = branch_node->original_next_node_id;
									node->next_node = branch_node->original_next_node;
								}
							}
							break;
						case NODE_TYPE_SCOPE:
							{
								ScopeNode* node = (ScopeNode*)inner_it->second;
								if (node->next_node == branch_node) {
									node->next_node_id = branch_node->original_next_node_id;
									node->next_node = branch_node->original_next_node;
								}
							}
							break;
						case NODE_TYPE_BRANCH:
							{
								BranchNode* node = (BranchNode*)inner_it->second;
								if (node->original_next_node == branch_node) {
									node->original_next_node_id = branch_node->original_next_node_id;
									node->original_next_node = branch_node->original_next_node;
								}
								if (node->branch_next_node == branch_node) {
									node->branch_next_node_id = branch_node->original_next_node_id;
									node->branch_next_node = branch_node->original_next_node;
								}
							}
							break;
						}
					}

					removed_node = true;

					delete it->second;
					it = this->nodes.erase(it);

					continue;
				}
			}

			it++;
		}

		if (!removed_node) {
			break;
		}
	}
}

void Scope::clean_node(int scope_id,
					   int node_id) {
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		if (it->second->type == NODE_TYPE_BRANCH) {
			BranchNode* branch_node = (BranchNode*)it->second;
			branch_node->clean_node(scope_id,
									node_id);
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
		}
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
}

ScopeHistory::ScopeHistory(ScopeHistory* original) {
	this->scope = original->scope;

	for (map<int, AbstractNodeHistory*>::iterator it = original->node_histories.begin();
			it != original->node_histories.end(); it++) {
		AbstractNode* node = this->scope->nodes[it->first];
		switch (node->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNodeHistory* original_history = (ActionNodeHistory*)it->second;
				this->node_histories[it->first] = new ActionNodeHistory(original_history);
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* original_history = (ScopeNodeHistory*)it->second;
				this->node_histories[it->first] = new ScopeNodeHistory(original_history);
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNodeHistory* original_history = (BranchNodeHistory*)it->second;
				this->node_histories[it->first] = new BranchNodeHistory(original_history);
			}
			break;
		}
	}
}

ScopeHistory::~ScopeHistory() {
	for (map<int, AbstractNodeHistory*>::iterator it = this->node_histories.begin();
			it != this->node_histories.end(); it++) {
		delete it->second;
	}
}