#include "scope.h"

#include <algorithm>
#include <iostream>
#include <sstream>

#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "network.h"
#include "obs_node.h"
#include "scope_node.h"
#include "solution.h"
#include "start_node.h"

using namespace std;

Scope::Scope() {
	// this->data_index = 0;

	// this->pre_network = NULL;
	// this->post_network = NULL;
	// this->long_iter = 0;
}

Scope::~Scope() {
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		delete it->second;
	}

	// if (this->pre_network != NULL) {
	// 	delete this->pre_network;
	// }

	// if (this->post_network != NULL) {
	// 	delete this->post_network;
	// }
}

void Scope::random_exit_activate(AbstractNode* starting_node,
								 vector<AbstractNode*>& possible_exits) {
	AbstractNode* curr_node = starting_node;
	while (true) {
		if (curr_node == NULL) {
			break;
		}

		switch (curr_node->type) {
		case NODE_TYPE_START:
			{
				StartNode* node = (StartNode*)curr_node;

				possible_exits.push_back(curr_node);

				curr_node = node->next_node;
			}
			break;
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

	// output_file << (this->pre_network == NULL) << endl;
	// if (this->pre_network != NULL) {
	// 	this->pre_network->save(output_file);
	// }
	// output_file << (this->post_network == NULL) << endl;
	// if (this->post_network != NULL) {
	// 	this->post_network->save(output_file);
	// }
	// output_file << this->long_iter << endl;
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

	// string pre_network_is_null_line;
	// getline(input_file, pre_network_is_null_line);
	// bool pre_network_is_null = stoi(pre_network_is_null_line);
	// if (pre_network_is_null) {
	// 	this->pre_network = NULL;
	// } else {
	// 	this->pre_network = new Network(input_file);
	// }

	// string post_network_is_null_line;
	// getline(input_file, post_network_is_null_line);
	// bool post_network_is_null = stoi(post_network_is_null_line);
	// if (post_network_is_null) {
	// 	this->post_network = NULL;
	// } else {
	// 	this->post_network = new Network(input_file);
	// }

	// string long_iter_line;
	// getline(input_file, long_iter_line);
	// this->long_iter = stoi(long_iter_line);
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
		case NODE_TYPE_START:
			{
				StartNode* original_start_node = (StartNode*)it->second;
				StartNode* start_node = new StartNode();
				start_node->parent = this;
				start_node->id = it->first;
				start_node->copy_from(original_start_node);
				this->nodes[it->first] = start_node;
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* original_action_node = (ActionNode*)it->second;
				ActionNode* action_node = new ActionNode();
				action_node->parent = this;
				action_node->id = it->first;
				action_node->copy_from(original_action_node);
				this->nodes[it->first] = action_node;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* original_scope_node = (ScopeNode*)it->second;
				ScopeNode* scope_node = new ScopeNode();
				scope_node->parent = this;
				scope_node->id = it->first;
				scope_node->copy_from(original_scope_node,
									  parent_solution);
				this->nodes[it->first] = scope_node;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* original_branch_node = (BranchNode*)it->second;
				BranchNode* branch_node = new BranchNode();
				branch_node->parent = this;
				branch_node->id = it->first;
				branch_node->copy_from(original_branch_node,
									   parent_solution);
				this->nodes[it->first] = branch_node;
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* original_obs_node = (ObsNode*)it->second;
				ObsNode* obs_node = new ObsNode();
				obs_node->parent = this;
				obs_node->id = it->first;
				obs_node->copy_from(original_obs_node,
									parent_solution);
				this->nodes[it->first] = obs_node;
			}
			break;
		}
	}

	for (int c_index = 0; c_index < (int)original->child_scopes.size(); c_index++) {
		this->child_scopes.push_back(parent_solution->scopes[
			original->child_scopes[c_index]->id]);
	}

	// this->pre_obs = original->pre_obs;
	// this->pre_targets = original->pre_targets;
	// this->post_obs = original->post_obs;
	// this->post_targets = original->post_targets;
	// this->data_index = original->data_index;

	// if (original->pre_network == NULL) {
	// 	this->pre_network = NULL;
	// } else {
	// 	this->pre_network = new Network(original->pre_network);
	// }

	// if (original->post_network == NULL) {
	// 	this->post_network = NULL;
	// } else {
	// 	this->post_network = new Network(original->post_network);
	// }

	// this->long_iter = original->long_iter;
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

ScopeHistory::ScopeHistory(ScopeHistory* original,
						   Solution* parent_solution) {
	this->scope = parent_solution->scopes[original->scope->id];

	for (map<int, AbstractNodeHistory*>::iterator it = original->node_histories.begin();
			it != original->node_histories.end(); it++) {
		AbstractNode* node = it->second->node;
		AbstractNodeHistory* new_node_history;
		switch (node->type) {
		case NODE_TYPE_START:
			{
				StartNode* start_node = (StartNode*)this->scope->nodes[it->first];
				new_node_history = new StartNodeHistory(start_node);
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)this->scope->nodes[it->first];
				new_node_history = new ActionNodeHistory(action_node);
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* original_scope_node_history = (ScopeNodeHistory*)it->second;

				ScopeNode* scope_node = (ScopeNode*)this->scope->nodes[it->first];
				ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(scope_node);
				scope_node_history->scope_history = new ScopeHistory(original_scope_node_history->scope_history,
																	 parent_solution);
				new_node_history = scope_node_history;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNodeHistory* original_branch_node_history = (BranchNodeHistory*)it->second;

				BranchNode* branch_node = (BranchNode*)this->scope->nodes[it->first];
				BranchNodeHistory* branch_node_history = new BranchNodeHistory(branch_node);
				branch_node_history->is_branch = original_branch_node_history->is_branch;
				new_node_history = branch_node_history;
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)this->scope->nodes[it->first];
				new_node_history = new ObsNodeHistory(obs_node);
			}
			break;
		}

		new_node_history->index = it->second->index;
		this->node_histories[it->first] = new_node_history;
	}
}

ScopeHistory::~ScopeHistory() {
	for (map<int, AbstractNodeHistory*>::iterator it = this->node_histories.begin();
			it != this->node_histories.end(); it++) {
		delete it->second;
	}
}
