#include "scope.h"

#include <iostream>

#include "abstract_node.h"
#include "action_node.h"
#include "branch_node.h"
#include "branch_stub_node.h"
#include "exit_node.h"
#include "globals.h"
#include "obs_experiment.h"
#include "scale.h"
#include "scope_node.h"
#include "solution.h"
#include "state.h"

using namespace std;

Scope::Scope() {
	this->obs_experiment = NULL;
}

Scope::~Scope() {
	for (int s_index = 0; s_index < this->num_input_states; s_index++) {
		delete this->input_state_scales[s_index];
	}
	for (int s_index = 0; s_index < this->num_local_states; s_index++) {
		delete this->local_state_scales[s_index];
	}

	for (map<State*, pair<Scale*, double>>::iterator it = this->score_state_scales.begin();
			it != this->score_state_scales.end(); it++) {
		delete it->second.first;
	}

	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		delete this->nodes[n_index];
	}

	if (this->obs_experiment != NULL) {
		delete this->obs_experiment;
	}
}

void Scope::save(ofstream& output_file) {
	output_file << this->num_input_states << endl;
	for (int s_index = 0; s_index < this->num_input_states; s_index++) {
		output_file << this->input_state_scales[s_index]->weight << endl;
	}

	output_file << this->num_local_states << endl;
	for (int s_index = 0; s_index < this->num_local_states; s_index++) {
		output_file << this->local_state_scales[s_index]->weight << endl;
	}

	output_file << this->score_state_scales.size() << endl;
	for (map<State*, pair<Scale*, double>>::iterator it = this->score_state_scales.begin();
			it != this->score_state_scales.end(); it++) {
		output_file << it->first->id << endl;
		output_file << it->second.first->weight << endl;
		output_file << it->second.second << endl;;
	}

	output_file << this->nodes.size() << endl;
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		output_file << this->nodes[n_index]->type << endl;
		this->nodes[n_index]->save(output_file);
	}

	output_file << this->average_score << endl;
	output_file << this->score_variance << endl;
	output_file << this->average_misguess << endl;
	output_file << this->misguess_variance << endl;

	output_file << this->child_scopes.size() << endl;
	for (int c_index = 0; c_index < (int)this->child_scopes.size(); c_index++) {
		output_file << this->child_scopes[c_index]->id << endl;
	}
}

void Scope::load(ifstream& input_file,
				 int id) {
	this->id = id;

	string num_input_states_line;
	getline(input_file, num_input_states_line);
	this->num_input_states = stoi(num_input_states_line);
	for (int s_index = 0; s_index < this->num_input_states; s_index++) {
		string weight_line;
		getline(input_file, weight_line);
		this->input_state_scales.push_back(new Scale(stod(weight_line)));
	}

	string num_local_states_line;
	getline(input_file, num_local_states_line);
	this->num_local_states = stoi(num_local_states_line);
	for (int s_index = 0; s_index < this->num_local_states; s_index++) {
		string weight_line;
		getline(input_file, weight_line);
		this->local_state_scales.push_back(new Scale(stod(weight_line)));
	}

	string num_score_states_line;
	getline(input_file, num_score_states_line);
	int num_score_states = stoi(num_score_states_line);
	for (int s_index = 0; s_index < num_score_states; s_index++) {
		string id_line;
		getline(input_file, id_line);
		State* state = solution->states[stoi(id_line)];

		string weight_line;
		getline(input_file, weight_line);

		string resolved_standard_deviation_line;
		getline(input_file, resolved_standard_deviation_line);

		this->score_state_scales[state] = {new Scale(stod(weight_line)), stod(resolved_standard_deviation_line)};

		this->score_state_nodes[state] = vector<AbstractNode*>(state->networks.size());
		/**
		 * - filled in on link_score_state_nodes()
		 */
	}

	string num_nodes_line;
	getline(input_file, num_nodes_line);
	int num_nodes = stoi(num_nodes_line);
	for (int n_index = 0; n_index < num_nodes; n_index++) {
		string type_line;
		getline(input_file, type_line);
		int type = stoi(type_line);
		if (type == NODE_TYPE_ACTION) {
			ActionNode* node = new ActionNode(input_file,
											  n_index);
			this->nodes.push_back(node);
		} else if (type == NODE_TYPE_SCOPE) {
			ScopeNode* node = new ScopeNode(input_file,
											n_index);
			this->nodes.push_back(node);
		} else if (type == NODE_TYPE_BRANCH) {
			BranchNode* node = new BranchNode(input_file,
											  n_index);
			this->nodes.push_back(node);
		} else if (type == NODE_TYPE_BRANCH_STUB) {
			BranchStubNode* node = new BranchStubNode(input_file,
													  n_index);
			this->nodes.push_back(node);
		} else {
			ExitNode* node = new ExitNode(input_file,
										  n_index);
			this->nodes.push_back(node);
		}
	}

	string average_score_line;
	getline(input_file, average_score_line);
	this->average_score = stod(average_score_line);

	string score_variance_line;
	getline(input_file, score_variance_line);
	this->score_variance = stod(score_variance_line);

	string average_misguess_line;
	getline(input_file, average_misguess_line);
	this->average_misguess = stod(average_misguess_line);

	string misguess_variance_line;
	getline(input_file, misguess_variance_line);
	this->misguess_variance = stod(misguess_variance_line);

	string child_scopes_size_line;
	getline(input_file, child_scopes_size_line);
	int child_scopes_size = stoi(child_scopes_size_line);
	for (int c_index = 0; c_index < child_scopes_size; c_index++) {
		string scope_id_line;
		getline(input_file, scope_id_line);
		this->child_scopes.push_back(solution->scopes[stoi(scope_id_line)]);
	}
}

void Scope::link_score_state_nodes() {
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		if (this->nodes[n_index]->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)this->nodes[n_index];

			for (int s_index = 0; s_index < (int)action_node->score_state_defs.size(); s_index++) {
				Scope* parent_scope = solution->scopes[action_node->score_state_scope_contexts[s_index][0]];
				parent_scope->score_state_nodes[action_node->score_state_defs[s_index]][action_node->score_state_network_indexes[s_index]] = action_node;
			}
		} else if (this->nodes[n_index]->type == NODE_TYPE_SCOPE) {
			ScopeNode* scope_node = (ScopeNode*)this->nodes[n_index];

			for (int s_index = 0; s_index < (int)scope_node->score_state_defs.size(); s_index++) {
				Scope* parent_scope = solution->scopes[scope_node->score_state_scope_contexts[s_index][0]];
				parent_scope->score_state_nodes[scope_node->score_state_defs[s_index]][scope_node->score_state_network_indexes[s_index]] = scope_node;
			}
		} else if (this->nodes[n_index]->type == NODE_TYPE_BRANCH) {
			BranchNode* branch_node = (BranchNode*)this->nodes[n_index];

			for (int s_index = 0; s_index < (int)branch_node->score_state_defs.size(); s_index++) {
				Scope* parent_scope = solution->scopes[branch_node->score_state_scope_contexts[s_index][0]];
				parent_scope->score_state_nodes[branch_node->score_state_defs[s_index]][branch_node->score_state_network_indexes[s_index]] = branch_node;
			}
		}
	}
}

void Scope::save_for_display(ofstream& output_file) {
	output_file << this->nodes.size() << endl;
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		output_file << this->nodes[n_index]->type << endl;
		this->nodes[n_index]->save_for_display(output_file);
	}
}

ScopeHistory::ScopeHistory(Scope* scope) {
	this->scope = scope;

	this->inner_branch_experiment_history = NULL;

	this->exceeded_depth = false;
}

ScopeHistory::~ScopeHistory() {
	for (int i_index = 0; i_index < (int)this->node_histories.size(); i_index++) {
		for (int h_index = 0; h_index < (int)this->node_histories[i_index].size(); h_index++) {
			delete this->node_histories[i_index][h_index];
		}
	}
}
