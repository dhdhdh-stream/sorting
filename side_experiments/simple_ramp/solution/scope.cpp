#include "scope.h"

#include <algorithm>
#include <iostream>

#include "abstract_experiment.h"
#include "action_node.h"
#include "branch_node.h"
#include "eval_experiment.h"
#include "globals.h"
#include "obs_node.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"
#include "start_node.h"

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

void Scope::random_exit_activate(AbstractNode* starting_node,
								 vector<AbstractNode*>& possible_exits) {
	AbstractNode* curr_node = starting_node;
	while (true) {
		if (curr_node == NULL) {
			break;
		}

		switch (curr_node->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* node = (ActionNode*)curr_node;

				possible_exits.push_back(curr_node);

				curr_node = node->next_node;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* node = (ScopeNode*)curr_node;

				possible_exits.push_back(curr_node);

				curr_node = node->next_node;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* node = (BranchNode*)curr_node;

				possible_exits.push_back(curr_node);

				uniform_int_distribution<int> distribution(0, 1);
				if (distribution(generator) == 0) {
					curr_node = node->branch_next_node;
				} else {
					curr_node = node->original_next_node;
				}
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* node = (ObsNode*)curr_node;

				possible_exits.push_back(curr_node);

				curr_node = node->next_node;
			}
			break;
		}
	}

	possible_exits.push_back(NULL);
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
		case NODE_TYPE_START:
			{
				StartNode* start_node = new StartNode();
				start_node->parent = this;
				start_node->id = id;
				start_node->load(input_file);
				this->nodes[start_node->id] = start_node;
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
