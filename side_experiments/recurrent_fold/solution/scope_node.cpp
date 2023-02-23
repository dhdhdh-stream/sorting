#include "scope_node.h"

using namespace std;

ScopeNode::ScopeNode(Scope* parent,
					 Scope* inner_scope,
					 vector<bool> inner_input_is_local,
					 vector<int> inner_input_indexes,
					 bool has_score_network,
					 Network* score_network) {
	this->parent = parent;
	this->inner_scope = inner_scope;
	this->inner_input_is_local = inner_input_is_local;
	this->inner_input_indexes = inner_input_indexes;
	this->has_score_network = has_score_network;
	this->score_network = score_network;
}

ScopeNode::~ScopeNode() {
	// TODO: track scopes in dictionary
	delete inner_scope;

	if (this->has_score_network) {
		delete this->score_network;
	}
}

int ScopeNode::activate(vector<double>& input_vals,
						vector<double>& local_state_vals,
						vector<vector<double>>& flat_vals,
						double& predicted_score) {
	vector<double> scope_input_vals;
	for (int i_index = 0; i_index < (int)this->inner_input_is_local.size(); i_index++) {
		if (this->inner_input_is_local[i_index]) {
			scope_input_vals.push_back(local_state_vals[this->inner_input_indexes[i_index]]);
		} else {
			scope_input_vals.push_back(input_vals[this->inner_input_indexes[i_index]]);
		}
	}
	// if inner_scope expanded so there's no matching index, pass 0.0
	for (int i_index = (int)this->inner_input_is_local.size(); i_index < this->inner_scope->num_input_states; i_index++) {
		scope_input_vals.push_back(0.0);
	}

	this->inner_scope->activate(scope_input_vals,
								flat_vals,
								predicted_score);

	for (int i_index = 0; i_index < (int)this->inner_input_is_local.size(); i_index++) {
		if (this->inner_input_is_local[i_index]) {
			local_state_vals[this->inner_input_indexes[i_index]] += scope_input_vals[i_index];
		} else {
			input_vals[this->inner_input_indexes[i_index]] += scope_input_vals[i_index];
		}
	}

	if (this->has_score_network) {
		this->score_network->scope_activate(input_vals,
											local_state_vals);
		predicted_score += this->score_network->output->acti_vals[0];
	}

	return this->next_node_index;
}
