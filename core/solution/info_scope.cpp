#include "info_scope.h"

#include <iostream>

#include "abstract_node.h"
#include "action_node.h"
#include "globals.h"
#include "info_scope_node.h"
#include "network.h"

using namespace std;

InfoScope::InfoScope() {
	this->type = SCOPE_TYPE_INFO;

	this->id = -1;

	this->network = NULL;

	this->experiment = NULL;

	#if defined(MDEBUG) && MDEBUG
	this->verify_key = NULL;
	#endif /* MDEBUG */
}

InfoScope::~InfoScope() {
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		delete this->nodes[n_index];
	}

	if (this->network != NULL) {
		delete this->network;
	}
}

#if defined(MDEBUG) && MDEBUG
void InfoScope::clear_verify() {
	this->verify_key = NULL;
	if (this->verify_scores.size() > 0) {
		cout << "seed: " << seed << endl;

		throw invalid_argument("info scope remaining verify");
	}
}
#endif /* MDEBUG */

void InfoScope::save(ofstream& output_file) {
	output_file << this->node_counter << endl;

	output_file << this->nodes.size() << endl;
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		output_file << it->first << endl;
		output_file << it->second->type << endl;
		it->second->save(output_file);
	}

	output_file << this->input_node_contexts.size() << endl;
	for (int i_index = 0; i_index < (int)this->input_node_contexts.size(); i_index++) {
		output_file << this->input_node_contexts[i_index]->id << endl;
		output_file << this->input_obs_indexes[i_index] << endl;
	}
	this->network->save(output_file);
}

void InfoScope::load(ifstream& input_file) {
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
		case NODE_TYPE_INFO_SCOPE:
			{
				InfoScopeNode* info_scope_node = new InfoScopeNode();
				info_scope_node->parent = this;
				info_scope_node->id = id;
				info_scope_node->load(input_file);
				this->nodes[info_scope_node->id] = info_scope_node;
			}
			break;
		}
	}

	string num_inputs_line;
	getline(input_file, num_inputs_line);
	int num_inputs = stoi(num_inputs_line);
	for (int i_index = 0; i_index < num_inputs; i_index++) {
		string node_context_id_line;
		getline(input_file, node_context_id_line);
		this->input_node_contexts.push_back(this->nodes[stoi(node_context_id_line)]);

		string obs_index_line;
		getline(input_file, obs_index_line);
		this->input_obs_indexes.push_back(stoi(obs_index_line));
	}
	this->network = new Network(input_file);
}

void InfoScope::link(Solution* parent_solution) {
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		it->second->link(parent_solution);
	}
}

void InfoScope::copy_from(InfoScope* original,
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
		case NODE_TYPE_INFO_SCOPE:
			{
				InfoScopeNode* original_info_scope_node = (InfoScopeNode*)it->second;
				InfoScopeNode* new_info_scope_node = new InfoScopeNode(
					original_info_scope_node,
					parent_solution);
				new_info_scope_node->parent = this;
				new_info_scope_node->id = it->first;
				this->nodes[it->first] = new_info_scope_node;
			}
			break;
		}
	}

	for (int i_index = 0; i_index < (int)original->input_node_contexts.size(); i_index++) {
		this->input_node_contexts.push_back(this->nodes[
			original->input_node_contexts[i_index]->id]);
	}
	this->input_obs_indexes = original->input_obs_indexes;
	this->network = new Network(original->network);
}

void InfoScope::save_for_display(ofstream& output_file) {
	output_file << this->nodes.size() << endl;
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		output_file << it->first << endl;
		output_file << it->second->type << endl;
		it->second->save_for_display(output_file);
	}
}

InfoScopeHistory::InfoScopeHistory(InfoScope* scope) {
	this->scope = scope;
}

InfoScopeHistory::~InfoScopeHistory() {
	for (map<AbstractNode*, AbstractNodeHistory*>::iterator it = this->node_histories.begin();
			it != this->node_histories.end(); it++) {
		delete it->second;
	}
}

AbstractScopeHistory* InfoScopeHistory::deep_copy() {
	InfoScopeHistory* new_scope_history = new InfoScopeHistory((InfoScope*)this->scope);

	for (map<AbstractNode*, AbstractNodeHistory*>::iterator it = this->node_histories.begin();
			it != this->node_histories.end(); it++) {
		switch (it->first->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
				new_scope_history->node_histories[it->first] = new ActionNodeHistory(action_node_history);
			}
			break;
		case NODE_TYPE_INFO_SCOPE:
			{
				InfoScopeNodeHistory* info_scope_node_history = (InfoScopeNodeHistory*)it->second;
				new_scope_history->node_histories[it->first] = new InfoScopeNodeHistory(info_scope_node_history);
			}
			break;
		}
	}

	return new_scope_history;
}
