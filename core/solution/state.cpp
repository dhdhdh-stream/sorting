#include "state.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "scale.h"
#include "scope.h"
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
}

State::~State() {
	for (int n_index = 0; n_index < (int)this->networks.size(); n_index++) {
		delete this->networks[n_index];
	}
}

void State::detach(Scope* parent_scope) {
	vector<AbstractNode*> nodes = parent_scope->score_state_nodes[this];

	for (int n_index = 0; n_index < (int)nodes.size(); n_index++) {
		if (nodes[n_index]->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)nodes[n_index];
			for (int s_index = 0; s_index < (int)action_node->score_state_defs.size(); s_index++) {
				if (action_node->score_state_defs[s_index] == this) {
					action_node->score_state_scope_contexts.erase(action_node->score_state_scope_contexts.begin() + s_index);
					action_node->score_state_node_contexts.erase(action_node->score_state_node_contexts.begin() + s_index);
					action_node->score_state_defs.erase(action_node->score_state_defs.begin() + s_index);
					action_node->score_state_network_indexes.erase(action_node->score_state_network_indexes.begin() + s_index);
					break;
				}
			}
		} else if (nodes[n_index]->type == NODE_TYPE_SCOPE) {
			ScopeNode* scope_node = (ScopeNode*)nodes[n_index];
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
			BranchNode* branch_node = (BranchNode*)nodes[n_index];
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

	delete parent_scope->score_state_scales[this].first;
	parent_scope->score_state_scales.erase(this);
	parent_scope->score_state_nodes.erase(this);

	solution->states.erase(this->id);
}

void State::save(ofstream& output_file) {
	output_file << this->networks.size() << endl;
	for (int n_index = 0; n_index < (int)this->networks.size(); n_index++) {
		ofstream network_save_file;
		network_save_file.open("saves/nns/" + to_string(this->id) + "_" + to_string(n_index) + ".txt");
		this->networks[n_index]->save(network_save_file);
		network_save_file.close();
	}
}
