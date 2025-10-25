#include "scope.h"

#include <algorithm>
#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "factor.h"
#include "globals.h"
#include "obs_node.h"
#include "scope_node.h"
#include "solution.h"
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

	for (int f_index = 0; f_index < (int)this->factors.size(); f_index++) {
		delete this->factors[f_index];
	}
}

void Scope::random_exit_activate(AbstractNode* starting_node,
								 vector<AbstractNode*>& possible_exits) {
	set<int> branch_node_ids_seen;

	AbstractNode* curr_node = starting_node;
	while (true) {
		if (curr_node == NULL) {
			break;
		}

		switch (curr_node->type) {
		case NODE_TYPE_START:
			{
				StartNode* node = (StartNode*)curr_node;

				if (branch_node_ids_seen.size() == 0) {
					possible_exits.push_back(curr_node);
				}

				curr_node = node->next_node;
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* node = (ActionNode*)curr_node;

				if (branch_node_ids_seen.size() == 0) {
					possible_exits.push_back(curr_node);
				}

				curr_node = node->next_node;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* node = (ScopeNode*)curr_node;

				if (branch_node_ids_seen.size() == 0) {
					possible_exits.push_back(curr_node);
				}

				curr_node = node->next_node;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* node = (BranchNode*)curr_node;

				if (branch_node_ids_seen.size() == 0) {
					possible_exits.push_back(curr_node);
				}

				branch_node_ids_seen.insert(node->id);

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

				if (branch_node_ids_seen.size() == 0) {
					possible_exits.push_back(curr_node);
				}

				curr_node = node->next_node;

				if (node->branch_start_node_id != -1) {
					set<int>::iterator it = branch_node_ids_seen.find(node->branch_start_node_id);
					if (it == branch_node_ids_seen.end()) {
						return;
					} else {
						branch_node_ids_seen.erase(it);
					}
				}
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
		}
	}

	for (int f_index = 0; f_index < (int)this->factors.size(); f_index++) {
		this->factors[f_index]->clean_inputs(scope,
											 node_id);
	}
}

void Scope::clean_inputs(Scope* scope) {
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		switch (it->second->type) {
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)it->second;
				branch_node->clean_inputs(scope);
			}
			break;
		}
	}

	for (int f_index = 0; f_index < (int)this->factors.size(); f_index++) {
		this->factors[f_index]->clean_inputs(scope);
	}
}

void Scope::replace_obs_node(Scope* scope,
							 int original_node_id,
							 int new_node_id) {
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		switch (it->second->type) {
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)it->second;
				branch_node->replace_obs_node(scope,
											  original_node_id,
											  new_node_id);
			}
			break;
		}
	}

	for (int f_index = 0; f_index < (int)this->factors.size(); f_index++) {
		this->factors[f_index]->replace_obs_node(scope,
												 original_node_id,
												 new_node_id,
												 f_index);
	}
}

void Scope::clean() {
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		it->second->clean();
	}
}

void Scope::measure_update(int total_count) {
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		it->second->measure_update(total_count);
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

	output_file << this->factors.size() << endl;
	for (int f_index = 0; f_index < (int)this->factors.size(); f_index++) {
		this->factors[f_index]->save(output_file);
	}

	output_file << this->child_scopes.size() << endl;
	for (set<Scope*>::iterator it = this->child_scopes.begin();
			it != this->child_scopes.end(); it++) {
		output_file << (*it)->id << endl;
		output_file << (*it)->is_external << endl;
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

	string num_factors_line;
	getline(input_file, num_factors_line);
	int num_factors = stoi(num_factors_line);
	for (int f_index = 0; f_index < num_factors; f_index++) {
		Factor* factor = new Factor();
		factor->load(input_file,
					 parent_solution);
		this->factors.push_back(factor);
	}

	string num_child_scopes_line;
	getline(input_file, num_child_scopes_line);
	int num_child_scopes = stoi(num_child_scopes_line);
	for (int c_index = 0; c_index < num_child_scopes; c_index++) {
		string scope_id_line;
		getline(input_file, scope_id_line);
		int scope_id = stoi(scope_id_line);

		string is_external_line;
		getline(input_file, is_external_line);
		bool is_external = stoi(is_external_line);

		if (is_external) {
			this->child_scopes.insert(parent_solution->external_scopes[scope_id]);
		} else {
			this->child_scopes.insert(parent_solution->scopes[scope_id]);
		}
	}
}

void Scope::link(Solution* parent_solution) {
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		it->second->link(parent_solution);
	}

	for (int f_index = 0; f_index < (int)this->factors.size(); f_index++) {
		this->factors[f_index]->link(f_index);
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

	this->factor_initialized = vector<bool>(scope->factors.size(), false);
	this->factor_values = vector<double>(scope->factors.size());
}

ScopeHistory::ScopeHistory(ScopeHistory* original) {
	this->scope = original->scope;

	for (map<int, AbstractNodeHistory*>::iterator it = original->node_histories.begin();
			it != original->node_histories.end(); it++) {
		switch (it->second->node->type) {
		case NODE_TYPE_START:
			{
				StartNodeHistory* start_node_history = (StartNodeHistory*)it->second;
				this->node_histories[it->first] = new StartNodeHistory(start_node_history);
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
				this->node_histories[it->first] = new ActionNodeHistory(action_node_history);
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
				this->node_histories[it->first] = new ScopeNodeHistory(scope_node_history);
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
				this->node_histories[it->first] = new BranchNodeHistory(branch_node_history);
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNodeHistory* obs_node_history = (ObsNodeHistory*)it->second;
				this->node_histories[it->first] = new ObsNodeHistory(obs_node_history);
			}
			break;
		}
	}

	this->factor_initialized = original->factor_initialized;
	this->factor_values = original->factor_values;

	this->num_actions_snapshot = original->num_actions_snapshot;
}

ScopeHistory::ScopeHistory(ScopeHistory* original,
						   int max_index,
						   int num_actions_snapshot) {
	this->scope = original->scope;

	for (map<int, AbstractNodeHistory*>::iterator it = original->node_histories.begin();
			it != original->node_histories.end(); it++) {
		if (it->second->index <= max_index) {
			switch (it->second->node->type) {
			case NODE_TYPE_START:
				{
					StartNodeHistory* start_node_history = (StartNodeHistory*)it->second;
					this->node_histories[it->first] = new StartNodeHistory(start_node_history);
				}
				break;
			case NODE_TYPE_ACTION:
				{
					ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
					this->node_histories[it->first] = new ActionNodeHistory(action_node_history);
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
					this->node_histories[it->first] = new ScopeNodeHistory(scope_node_history);
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
					this->node_histories[it->first] = new BranchNodeHistory(branch_node_history);
				}
				break;
			case NODE_TYPE_OBS:
				{
					ObsNodeHistory* obs_node_history = (ObsNodeHistory*)it->second;
					this->node_histories[it->first] = new ObsNodeHistory(obs_node_history);
				}
				break;
			}
		}
	}

	this->factor_initialized = vector<bool>(scope->factors.size(), false);
	this->factor_values = vector<double>(scope->factors.size());

	this->num_actions_snapshot = num_actions_snapshot;
}

ScopeHistory::~ScopeHistory() {
	for (map<int, AbstractNodeHistory*>::iterator it = this->node_histories.begin();
			it != this->node_histories.end(); it++) {
		delete it->second;
	}
}
