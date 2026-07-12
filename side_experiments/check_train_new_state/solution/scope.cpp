#include "scope.h"

#include <algorithm>
#include <iostream>
#include <sstream>

#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "network.h"
#include "noop_node.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

Scope::Scope() {
	// do nothing
}

Scope::~Scope() {
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		delete it->second;
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

	output_file << this->last_scores.size() << endl;
	for (list<double>::iterator it = this->last_scores.begin();
			it != this->last_scores.end(); it++) {
		output_file << *it << endl;
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
		case NODE_TYPE_NOOP:
			{
				NoopNode* noop_node = new NoopNode();
				noop_node->parent = this;
				noop_node->id = id;
				noop_node->load(input_file,
								parent_solution);
				this->nodes[noop_node->id] = noop_node;
			}
			break;
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
				branch_node->load(input_file,
								  parent_solution);
				this->nodes[branch_node->id] = branch_node;
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

	string num_last_scores_line;
	getline(input_file, num_last_scores_line);
	int num_last_scores = stoi(num_last_scores_line);
	for (int e_index = 0; e_index < num_last_scores; e_index++) {
		string score_line;
		getline(input_file, score_line);
		this->last_scores.push_back(stod(score_line));
	}
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
