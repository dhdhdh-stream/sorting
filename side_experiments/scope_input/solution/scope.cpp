#include "scope.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "factor.h"
#include "obs_node.h"
#include "scope_node.h"
#include "solution.h"

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

void Scope::clean_inputs(Scope* scope,
						 int node_id) {
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		switch (it->second->type) {
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)it->second;
				branch_node->clean_inputs(scope,
										  node_id);
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)it->second;
				obs_node->clean_inputs(scope,
									   node_id);
			}
			break;
		}
	}

	for (map<Scope*, vector<Input>>::iterator it = this->child_scope_inputs.begin();
			it != this->child_scope_inputs.end(); it++) {
		for (int i_index = 0; i_index < (int)it->second.size(); i_index++) {
			for (int l_index = 0; l_index < (int)it->second[i_index].scope_context.size(); l_index++) {
				if (it->second[i_index].scope_context[l_index] == scope
						&& it->second[i_index].node_context[l_index] == node_id) {
					it->second[i_index].type = INPUT_TYPE_REMOVED;
					it->second[i_index].scope_context.clear();
					it->second[i_index].node_context.clear();
					it->second[i_index].factor_index = -1;
					it->second[i_index].obs_index = -1;
					it->second[i_index].input_index = -1;
				}
			}
		}
	}
}

void Scope::clean_inputs(Scope* scope) {
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		switch (it->second->type) {
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)it->second;
				obs_node->clean_inputs(scope);
			}
			break;
		}
	}

	for (int c_index = 0; c_index < (int)this->child_scopes.size(); c_index++) {
		if (this->child_scopes[c_index] == scope) {
			this->child_scopes.erase(this->child_scopes.begin() + c_index);
			this->child_scope_inputs.erase(scope);
			break;
		}
	}

	for (map<Scope*, vector<Input>>::iterator it = this->child_scope_inputs.begin();
			it != this->child_scope_inputs.end(); it++) {
		for (int i_index = 0; i_index < (int)it->second.size(); i_index++) {
			for (int l_index = 0; l_index < (int)it->second[i_index].scope_context.size(); l_index++) {
				if (it->second[i_index].scope_context[l_index] == scope) {
					it->second[i_index].type = INPUT_TYPE_REMOVED;
					it->second[i_index].scope_context.clear();
					it->second[i_index].node_context.clear();
					it->second[i_index].factor_index = -1;
					it->second[i_index].obs_index = -1;
					it->second[i_index].input_index = -1;
				}
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

	output_file << this->num_inputs << endl;

	output_file << this->child_scopes.size() << endl;
	for (int c_index = 0; c_index < (int)this->child_scopes.size(); c_index++) {
		output_file << this->child_scopes[c_index]->id << endl;
	}
	for (map<Scope*, vector<Input>>::iterator it = this->child_scope_inputs.begin();
			it != this->child_scope_inputs.end(); it++) {
		output_file << it->first->id << endl;

		for (int i_index = 0; i_index < (int)it->second.size(); i_index++) {
			it->second[i_index].save(output_file);
		}
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

	string num_inputs_line;
	getline(input_file, num_inputs_line);
	this->num_inputs = stoi(num_inputs_line);

	string num_child_scopes_line;
	getline(input_file, num_child_scopes_line);
	int num_child_scopes = stoi(num_child_scopes_line);
	for (int c_index = 0; c_index < num_child_scopes; c_index++) {
		string scope_id_line;
		getline(input_file, scope_id_line);
		this->child_scopes.push_back(parent_solution->scopes[stoi(scope_id_line)]);
	}
	for (int c_index = 0; c_index < num_child_scopes; c_index++) {
		string scope_id_line;
		getline(input_file, scope_id_line);
		int scope_id = stoi(scope_id_line);

		vector<Input> inputs;
		for (int i_index = 0; i_index < num_inputs; i_index++) {
			inputs.push_back(Input(input_file,
								   parent_solution));
		}

		this->child_scope_inputs[parent_solution->scopes[scope_id]] = inputs;
	}
}

void Scope::link(Solution* parent_solution) {
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		it->second->link(parent_solution);
	}

	for (map<Scope*, vector<Input>>::iterator it = this->child_scope_inputs.begin();
			it != this->child_scope_inputs.end(); it++) {
		for (int i_index = 0; i_index < (int)it->second.size(); i_index++) {
			if (it->second[i_index].scope_context.size() > 0) {
				Scope* scope = it->second[i_index].scope_context.back();
				AbstractNode* node = scope->nodes[it->second[i_index].node_context.back()];
				switch (node->type) {
				case NODE_TYPE_BRANCH:
					{
						BranchNode* branch_node = (BranchNode*)node;
						branch_node->is_used = true;
					}
					break;
				case NODE_TYPE_OBS:
					{
						ObsNode* obs_node = (ObsNode*)node;

						if (it->second[i_index].factor_index != -1) {
							Factor* factor = obs_node->factors[it->second[i_index].factor_index];

							factor->link(parent_solution);
						}

						obs_node->is_used = true;
					}
					break;
				}
			}
		}
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
				BranchNode* new_branch_node = new BranchNode(original_branch_node,
															 parent_solution);
				new_branch_node->parent = this;
				new_branch_node->id = it->first;
				this->nodes[it->first] = new_branch_node;
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* original_obs_node = (ObsNode*)it->second;
				ObsNode* new_obs_node = new ObsNode(original_obs_node,
													parent_solution);
				new_obs_node->parent = this;
				new_obs_node->id = it->first;
				this->nodes[it->first] = new_obs_node;
			}
			break;
		}
	}

	this->num_inputs = original->num_inputs;

	for (int c_index = 0; c_index < (int)original->child_scopes.size(); c_index++) {
		this->child_scopes.push_back(parent_solution->scopes[
			original->child_scopes[c_index]->id]);
	}
	for (map<Scope*, vector<Input>>::iterator it = original->child_scope_inputs.begin();
			it != original->child_scope_inputs.end(); it++) {
		Scope* new_scope = parent_solution->scopes[it->first->id];

		vector<Input> new_inputs;
		for (int i_index = 0; i_index < (int)it->second.size(); i_index++) {
			new_inputs.push_back(Input(it->second[i_index],
									   parent_solution));
		}

		this->child_scope_inputs[new_scope] = new_inputs;
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
