#include "scope.h"

#include <iostream>

#include "abstract_experiment.h"
#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_scope.h"
#include "network.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

Scope::Scope() {
	this->type = SCOPE_TYPE_SCOPE;

	this->id = -1;

	this->eval_network = NULL;

	this->new_action_experiment = NULL;
}

Scope::~Scope() {
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		delete this->nodes[n_index];
	}

	if (this->eval_network != NULL) {
		delete this->eval_network;
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

	for (int i_index = this->eval_input_node_contexts.size()-1; i_index >= 0; i_index--) {
		bool has_match = false;
		for (int l_index = 0; l_index < (int)this->eval_input_scope_contexts[i_index].size(); l_index++) {
			if (this->eval_input_scope_context_ids[i_index][l_index] == scope_id
					&& this->eval_input_node_context_ids[i_index][l_index] == node_id) {
				has_match = true;
				break;
			}
		}

		if (has_match) {
			this->eval_input_scope_context_ids.erase(this->eval_input_scope_context_ids.begin() + i_index);
			this->eval_input_scope_contexts.erase(this->eval_input_scope_contexts.begin() + i_index);
			this->eval_input_node_context_ids.erase(this->eval_input_node_context_ids.begin() + i_index);
			this->eval_input_node_contexts.erase(this->eval_input_node_contexts.begin() + i_index);
			this->eval_input_obs_indexes.erase(this->eval_input_obs_indexes.begin() + i_index);
			this->eval_network->remove_input(i_index);
		}
	}
}

void Scope::update_structure() {
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		switch (it->second->type) {
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)it->second;
				branch_node->update_structure();
			}
			break;
		}
	}

	for (int i_index = 0; i_index < (int)this->eval_input_scope_contexts.size(); i_index++) {
		for (int l_index = 0; l_index < (int)this->eval_input_scope_contexts[i_index].size(); l_index++) {
			this->eval_input_scope_context_ids[i_index][l_index] = this->eval_input_scope_contexts[i_index][l_index]->id;
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

	bool network_is_null = this->eval_network == NULL;
	output_file << network_is_null << endl;
	if (!network_is_null) {
		output_file << this->eval_input_scope_contexts.size() << endl;
		for (int i_index = 0; i_index < (int)this->eval_input_scope_contexts.size(); i_index++) {
			output_file << this->eval_input_scope_contexts[i_index].size() << endl;
			for (int c_index = 0; c_index < (int)this->eval_input_scope_contexts[i_index].size(); c_index++) {
				output_file << this->eval_input_scope_context_ids[i_index][c_index] << endl;
				output_file << this->eval_input_node_context_ids[i_index][c_index] << endl;
			}
			output_file << this->eval_input_obs_indexes[i_index] << endl;
		}

		this->eval_network->save(output_file);
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
		case NODE_TYPE_INFO_BRANCH:
			{
				InfoBranchNode* info_branch_node = new InfoBranchNode();
				info_branch_node->parent = this;
				info_branch_node->id = id;
				info_branch_node->load(input_file,
									   parent_solution);
				this->nodes[info_branch_node->id] = info_branch_node;
			}
			break;
		}
	}

	string network_is_null_line;
	getline(input_file, network_is_null_line);
	bool network_is_null = stoi(network_is_null_line);
	if (!network_is_null) {
		string eval_num_inputs_line;
		getline(input_file, eval_num_inputs_line);
		int eval_num_inputs = stoi(eval_num_inputs_line);
		for (int i_index = 0; i_index < eval_num_inputs; i_index++) {
			string context_size_line;
			getline(input_file, context_size_line);
			int context_size = stoi(context_size_line);
			vector<int> c_scope_context_ids;
			vector<int> c_node_context_ids;
			for (int c_index = 0; c_index < context_size; c_index++) {
				string scope_context_id;
				getline(input_file, scope_context_id);
				c_scope_context_ids.push_back(stoi(scope_context_id));

				string node_context_id;
				getline(input_file, node_context_id);
				c_node_context_ids.push_back(stoi(node_context_id));
			}
			this->eval_input_scope_context_ids.push_back(c_scope_context_ids);
			this->eval_input_node_context_ids.push_back(c_node_context_ids);

			string obs_index_line;
			getline(input_file, obs_index_line);
			this->eval_input_obs_indexes.push_back(stoi(obs_index_line));
		}

		this->eval_network = new Network(input_file);
	}
}

void Scope::link(Solution* parent_solution) {
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		it->second->link(parent_solution);
	}

	for (int i_index = 0; i_index < (int)this->eval_input_scope_context_ids.size(); i_index++) {
		vector<AbstractScope*> c_scope_context;
		vector<AbstractNode*> c_node_context;
		for (int c_index = 0; c_index < (int)this->eval_input_scope_context_ids[i_index].size(); c_index++) {
			int scope_id = this->eval_input_scope_context_ids[i_index][c_index];
			Scope* scope = parent_solution->scopes[scope_id];
			c_scope_context.push_back(scope);
			c_node_context.push_back(scope->nodes[this->eval_input_node_context_ids[i_index][c_index]]);
		}
		this->eval_input_scope_contexts.push_back(c_scope_context);
		this->eval_input_node_contexts.push_back(c_node_context);
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
		case NODE_TYPE_INFO_BRANCH:
			{
				InfoBranchNode* original_info_branch_node = (InfoBranchNode*)it->second;
				InfoBranchNode* new_info_branch_node = new InfoBranchNode(
					original_info_branch_node,
					parent_solution);
				new_info_branch_node->parent = this;
				new_info_branch_node->id = it->first;
				this->nodes[it->first] = new_info_branch_node;
			}
			break;
		}
	}

	this->eval_input_scope_context_ids = original->eval_input_scope_context_ids;
	this->eval_input_node_context_ids = original->eval_input_node_context_ids;
	this->eval_input_obs_indexes = original->eval_input_obs_indexes;
	if (original->eval_network == NULL) {
		this->eval_network = NULL;
	} else {
		this->eval_network = new Network(original->eval_network);
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

	this->callback_experiment_history = NULL;
}

ScopeHistory::~ScopeHistory() {
	for (map<AbstractNode*, AbstractNodeHistory*>::iterator it = this->node_histories.begin();
			it != this->node_histories.end(); it++) {
		delete it->second;
	}
}

AbstractScopeHistory* ScopeHistory::deep_copy() {
	ScopeHistory* new_scope_history = new ScopeHistory((Scope*)this->scope);

	for (map<AbstractNode*, AbstractNodeHistory*>::iterator it = this->node_histories.begin();
			it != this->node_histories.end(); it++) {
		switch (it->first->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
				new_scope_history->node_histories[it->first] = new ActionNodeHistory(action_node_history);
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
				new_scope_history->node_histories[it->first] = new ScopeNodeHistory(scope_node_history);
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
				new_scope_history->node_histories[it->first] = new BranchNodeHistory(branch_node_history);
			}
			break;
		case NODE_TYPE_INFO_BRANCH:
			{
				InfoBranchNodeHistory* info_branch_node_history = (InfoBranchNodeHistory*)it->second;
				new_scope_history->node_histories[it->first] = new InfoBranchNodeHistory(info_branch_node_history);
			}
			break;
		}
	}

	return new_scope_history;
}
