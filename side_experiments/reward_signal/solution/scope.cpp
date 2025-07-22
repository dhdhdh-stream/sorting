#include "scope.h"

#include <algorithm>
#include <iostream>

#include "abstract_experiment.h"
#include "action_node.h"
#include "branch_node.h"
#include "factor.h"
#include "globals.h"
#include "obs_node.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

Scope::Scope() {
	this->score_average_val = 0.0;
}

Scope::~Scope() {
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		delete this->nodes[n_index];
	}

	for (int f_index = 0; f_index < (int)this->factors.size(); f_index++) {
		delete this->factors[f_index];
	}
}

void Scope::invalidate_factor(ScopeHistory* scope_history,
							  int f_index) {
	scope_history->factor_initialized[f_index] = false;

	for (int if_index = 0; if_index < (int)this->factors[f_index]->impacted_factors.size(); if_index++) {
		invalidate_factor(scope_history,
						  this->factors[f_index]->impacted_factors[if_index]);
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

	for (int i_index = (int)this->score_inputs.size()-1; i_index >= 0; i_index--) {
		bool is_match = false;
		for (int l_index = 0; l_index < (int)this->score_inputs[i_index].scope_context.size(); l_index++) {
			if (this->score_inputs[i_index].scope_context[l_index] == scope
					&& this->score_inputs[i_index].node_context[l_index] == node_id) {
				is_match = true;
				break;
			}
		}

		if (is_match) {
			this->score_inputs.erase(this->score_inputs.begin() + i_index);
			this->score_input_averages.erase(this->score_input_averages.begin() + i_index);
			this->score_input_standard_deviations.erase(this->score_input_standard_deviations.begin() + i_index);
			this->score_weights.erase(this->score_weights.begin() + i_index);
		}
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

	for (int i_index = (int)this->score_inputs.size()-1; i_index >= 0; i_index--) {
		bool is_match = false;
		for (int l_index = 0; l_index < (int)this->score_inputs[i_index].scope_context.size(); l_index++) {
			if (this->score_inputs[i_index].scope_context[l_index] == scope) {
				is_match = true;
				break;
			}
		}

		if (is_match) {
			this->score_inputs.erase(this->score_inputs.begin() + i_index);
			this->score_input_averages.erase(this->score_input_averages.begin() + i_index);
			this->score_input_standard_deviations.erase(this->score_input_standard_deviations.begin() + i_index);
			this->score_weights.erase(this->score_weights.begin() + i_index);
		}
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

	for (int i_index = 0; i_index < (int)this->score_inputs.size(); i_index++) {
		if (this->score_inputs[i_index].scope_context.back() == scope
				&& this->score_inputs[i_index].node_context.back() == original_node_id) {
			this->score_inputs[i_index].node_context.back() = new_node_id;
		}
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

	output_file << this->score_average_val << endl;
	output_file << this->score_inputs.size() << endl;
	for (int i_index = 0; i_index < (int)this->score_inputs.size(); i_index++) {
		this->score_inputs[i_index].save(output_file);
		output_file << this->score_input_averages[i_index] << endl;
		output_file << this->score_input_standard_deviations[i_index] << endl;
		output_file << this->score_weights[i_index] << endl;
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

	string score_average_val_line;
	getline(input_file, score_average_val_line);
	this->score_average_val = stod(score_average_val_line);

	string score_num_inputs_line;
	getline(input_file, score_num_inputs_line);
	int score_num_inputs = stoi(score_num_inputs_line);
	for (int i_index = 0; i_index < score_num_inputs; i_index++) {
		this->score_inputs.push_back(Input(input_file,
										   parent_solution));

		string input_average_line;
		getline(input_file, input_average_line);
		this->score_input_averages.push_back(stod(input_average_line));

		string input_standard_deviation_line;
		getline(input_file, input_standard_deviation_line);
		this->score_input_standard_deviations.push_back(stod(input_standard_deviation_line));

		string weight_line;
		getline(input_file, weight_line);
		this->score_weights.push_back(stod(weight_line));
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

	this->has_explore = false;

	this->signal_initialized = false;
}

ScopeHistory::ScopeHistory(ScopeHistory* original) {
	this->scope = original->scope;

	for (map<int, AbstractNodeHistory*>::iterator it = original->node_histories.begin();
			it != original->node_histories.end(); it++) {
		switch (it->second->node->type) {
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

	this->has_explore = original->has_explore;

	this->signal_initialized = false;
}

ScopeHistory::ScopeHistory(ScopeHistory* original,
						   int max_index) {
	this->scope = original->scope;

	for (map<int, AbstractNodeHistory*>::iterator it = original->node_histories.begin();
			it != original->node_histories.end(); it++) {
		if (it->second->index <= max_index) {
			switch (it->second->node->type) {
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

	this->has_explore = original->has_explore;

	this->signal_initialized = false;
}

ScopeHistory::~ScopeHistory() {
	for (map<int, AbstractNodeHistory*>::iterator it = this->node_histories.begin();
			it != this->node_histories.end(); it++) {
		delete it->second;
	}
}
