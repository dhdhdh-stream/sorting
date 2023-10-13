#include "state.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "scale.h"
#include "scope_node.h"
#include "solution.h"
#include "state_network.h"

using namespace std;

State::State() {
	// do nothing
}

State::State(ifstream& input_file,
			 int id) {
	this->id = id;

	string num_networks_line;
	getline(input_file, num_networks_line);
	int num_networks = stoi(num_networks_line);
	for (int n_index = 0; n_index < num_networks; n_index++) {
		ifstream network_save_file;
		network_save_file.open("saves/nns/" + to_string(this->id) + "_" + to_string(n_index) + ".txt");
		this->networks.push_back(new StateNetwork(network_save_file,
												  this,
												  n_index));
		network_save_file.close();
	}

	string resolved_network_indexes_size_line;
	getline(input_file, resolved_network_indexes_size_line);
	int resolved_network_indexes_size = stoi(resolved_network_indexes_size_line);
	for (int i_index = 0; i_index < resolved_network_indexes_size; i_index++) {
		string index_line;
		getline(input_file, index_line);
		this->resolved_network_indexes.insert(stoi(index_line));
	}

	string resolved_standard_deviation_line;
	getline(input_file, resolved_standard_deviation_line);
	this->resolved_standard_deviation = stod(resolved_standard_deviation_line);

	string scale_weight_line;
	getline(input_file, scale_weight_line);
	this->scale = new Scale(stod(scale_weight_line));

	this->nodes = vector<AbstractNode*>(this->networks.size());
	/**
	 * - filled in when nodes are loaded
	 */
}

State::~State() {
	for (int n_index = 0; n_index < (int)this->networks.size(); n_index++) {
		delete this->networks[n_index];
	}

	delete this->scale;
}

void State::detach() {
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		if (this->nodes[n_index]->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)this->nodes[n_index];
			for (int s_index = 0; s_index < (int)action_node->score_state_defs.size(); s_index++) {
				if (action_node->score_state_defs[s_index] == this) {
					action_node->score_state_scope_contexts.erase(action_node->score_state_scope_contexts.begin() + s_index);
					action_node->score_state_node_contexts.erase(action_node->score_state_node_contexts.begin() + s_index);
					action_node->score_state_defs.erase(action_node->score_state_defs.begin() + s_index);
					action_node->score_state_network_indexes.erase(action_node->score_state_network_indexes.begin() + s_index);
					break;
				}
			}
		} else if (this->nodes[n_index]->type == NODE_TYPE_SCOPE) {
			ScopeNode* scope_node = (ScopeNode*)this->nodes[n_index];
			for (int s_index = 0; s_index < (int)scope_node->score_state_defs.size(); s_index++) {
				if (scope_node->score_state_defs[s_index] == this) {
					scope_node->score_state_scope_contexts.erase(scope_node->score_state_scope_contexts.begin() + s_index);
					scope_node->score_state_node_contexts.erase(scope_node->score_state_node_contexts.begin() + s_index);
					scope_node->score_state_obs_indexes.erase(scope_node->score_state_obs_indexes.begin() + s_index);
					scope_node->score_state_defs.erase(scope_node->score_state_defs.begin() + s_index);
					scope_node->score_state_network_indexes.erase(scope_node->score_state_network_indexes.begin() + s_index);
					break;
				}
			}
		} else {
			BranchNode* branch_node = (BranchNode*)this->nodes[n_index];
			for (int s_index = 0; s_index < (int)branch_node->score_state_defs.size(); s_index++) {
				if (branch_node->score_state_defs[s_index] == this) {
					branch_node->score_state_scope_contexts.erase(branch_node->score_state_scope_contexts.begin() + s_index);
					branch_node->score_state_node_contexts.erase(branch_node->score_state_node_contexts.begin() + s_index);
					branch_node->score_state_defs.erase(branch_node->score_state_defs.begin() + s_index);
					branch_node->score_state_network_indexes.erase(branch_node->score_state_network_indexes.begin() + s_index);
					break;
				}
			}
		}
	}
}

void State::save(ofstream& output_file) {
	output_file << this->networks.size() << endl;
	for (int n_index = 0; n_index < (int)this->networks.size(); n_index++) {
		ofstream network_save_file;
		network_save_file.open("saves/nns/" + to_string(this->id) + "_" + to_string(n_index) + ".txt");
		this->networks[n_index]->save(network_save_file);
		network_save_file.close();
	}

	output_file << this->resolved_network_indexes.size() << endl;
	for (set<int>::iterator it = this->resolved_network_indexes.begin();
			it != this->resolved_network_indexes.end(); it++) {
		output_file << *it << endl;
	}

	output_file << this->resolved_standard_deviation << endl;

	output_file << this->scale->weight << endl;
}
