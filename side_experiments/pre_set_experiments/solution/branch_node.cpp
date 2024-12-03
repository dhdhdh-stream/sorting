#include "branch_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "action_node.h"
#include "globals.h"
#include "network.h"
#include "scope.h"
#include "solution.h"

using namespace std;

BranchNode::BranchNode() {
	this->type = NODE_TYPE_BRANCH;

	this->network = NULL;

	this->is_experiment = false;
	this->experiment = NULL;

	this->average_instances_per_run = 0.0;

	#if defined(MDEBUG) && MDEBUG
	this->verify_key = NULL;
	#endif /* MDEBUG */
}

BranchNode::BranchNode(BranchNode* original) {
	this->type = NODE_TYPE_BRANCH;

	this->inputs = original->inputs;
	this->network = new Network(original->network);

	this->original_next_node_id = original->original_next_node_id;
	this->branch_next_node_id = original->branch_next_node_id;

	this->is_experiment = false;
	this->experiment = NULL;

	this->average_instances_per_run = 0.0;

	#if defined(MDEBUG) && MDEBUG
	this->verify_key = NULL;
	#endif /* MDEBUG */
}

BranchNode::~BranchNode() {
	if (this->network != NULL) {
		delete this->network;
	}

	if (this->experiment != NULL) {
		delete this->experiment;
	}
}

#if defined(MDEBUG) && MDEBUG
void BranchNode::clear_verify() {
	this->verify_key = NULL;
	if (this->verify_scores.size() > 0) {
		cout << "seed: " << seed << endl;

		throw invalid_argument("branch node remaining verify");
	}
}
#endif /* MDEBUG */

void BranchNode::clean_inputs(Scope* scope,
							  int node_id) {
	for (int i_index = (int)this->inputs.size()-1; i_index >= 0; i_index--) {
		bool is_match = false;
		for (int l_index = 0; l_index < (int)this->inputs[i_index].first.first.size(); l_index++) {
			if (this->inputs[i_index].first.first[l_index] == scope
					&& this->inputs[i_index].first.second[l_index] == node_id) {
				is_match = true;
				break;
			}
		}

		if (is_match) {
			this->inputs.erase(this->inputs.begin() + i_index);
			this->network->remove_input(i_index);
		}
	}

	for (int i_index = (int)this->input_scope_contexts.size()-1; i_index >= 0; i_index--) {
		bool is_match = false;
		for (int l_index = 0; l_index < (int)this->input_scope_contexts[i_index].size(); l_index++) {
			if (this->input_scope_contexts[i_index][l_index] == scope
					&& this->input_node_context_ids[i_index][l_index] == node_id) {
				is_match = true;
				break;
			}
		}

		if (is_match) {
			this->input_scope_contexts.erase(this->input_scope_contexts.begin() + i_index);
			this->input_node_context_ids.erase(this->input_node_context_ids.begin() + i_index);
		}
	}
}

void BranchNode::clean_inputs(Scope* scope) {
	for (int i_index = (int)this->inputs.size()-1; i_index >= 0; i_index--) {
		bool is_match = false;
		for (int l_index = 0; l_index < (int)this->inputs[i_index].first.first.size(); l_index++) {
			if (this->inputs[i_index].first.first[l_index] == scope) {
				is_match = true;
				break;
			}
		}

		if (is_match) {
			this->inputs.erase(this->inputs.begin() + i_index);
			this->network->remove_input(i_index);
		}
	}

	for (int i_index = (int)this->input_scope_contexts.size()-1; i_index >= 0; i_index--) {
		bool is_match = false;
		for (int l_index = 0; l_index < (int)this->input_scope_contexts[i_index].size(); l_index++) {
			if (this->input_scope_contexts[i_index][l_index] == scope) {
				is_match = true;
				break;
			}
		}

		if (is_match) {
			this->input_scope_contexts.erase(this->input_scope_contexts.begin() + i_index);
			this->input_node_context_ids.erase(this->input_node_context_ids.begin() + i_index);
		}
	}
}

void BranchNode::save(ofstream& output_file) {
	output_file << this->inputs.size() << endl;
	for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
		output_file << this->inputs[i_index].first.first.size() << endl;
		for (int l_index = 0; l_index < (int)this->inputs[i_index].first.first.size(); l_index++) {
			output_file << this->inputs[i_index].first.first[l_index] << endl;
			output_file << this->inputs[i_index].first.second[l_index] << endl;
		}

		output_file << this->inputs[i_index].second << endl;
	}

	this->network->save(output_file);

	output_file << this->original_next_node_id << endl;
	output_file << this->branch_next_node_id << endl;

	output_file << this->is_experiment << endl;
	output_file << this->experiment_is_branch << endl;

	output_file << this->average_instances_per_run << endl;
}

void BranchNode::load(ifstream& input_file,
					  Solution* parent_solution) {
	{
		string num_inputs_line;
		getline(input_file, num_inputs_line);
		int num_inputs = stoi(num_inputs_line);
		for (int i_index = 0; i_index < num_inputs; i_index++) {
			vector<Scope*> scopes;
			vector<int> node_ids;

			string num_layers_line;
			getline(input_file, num_layers_line);
			int num_layers = stoi(num_layers_line);
			for (int l_index = 0; l_index < num_layers; l_index++) {
				string scope_id_line;
				getline(input_file, scope_id_line);
				scopes.push_back(parent_solution->scopes[stoi(scope_id_line)]);

				string node_id_line;
				getline(input_file, node_id_line);
				node_ids.push_back(stoi(node_id_line));
			}

			string obs_index_line;
			getline(input_file, obs_index_line);
			int obs_index = stoi(obs_index_line);

			this->inputs.push_back({{scopes, node_ids}, obs_index});
		}
	}

	this->network = new Network(input_file);

	string original_next_node_id_line;
	getline(input_file, original_next_node_id_line);
	this->original_next_node_id = stoi(original_next_node_id_line);

	string branch_next_node_id_line;
	getline(input_file, branch_next_node_id_line);
	this->branch_next_node_id = stoi(branch_next_node_id_line);

	string is_experiment_line;
	getline(input_file, is_experiment_line);
	this->is_experiment = stoi(is_experiment_line);

	string experiment_is_branch_line;
	getline(input_file, experiment_is_branch_line);
	this->experiment_is_branch = stoi(experiment_is_branch_line);

	string average_instances_per_run_line;
	getline(input_file, average_instances_per_run_line);
	this->average_instances_per_run = stod(average_instances_per_run_line);
}

void BranchNode::link(Solution* parent_solution) {
	for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
		Scope* scope = this->inputs[i_index].first.first.back();
		AbstractNode* node = scope->nodes[this->inputs[i_index].first.second.back()];
		switch (node->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* input_action_node = (ActionNode*)node;

				bool is_existing = false;
				for (int ii_index = 0; ii_index < (int)input_action_node->input_scope_contexts.size(); ii_index++) {
					if (input_action_node->input_scope_contexts[ii_index] == this->inputs[i_index].first.first
							&& input_action_node->input_node_context_ids[ii_index] == this->inputs[i_index].first.second
							&& input_action_node->input_obs_indexes[ii_index] == this->inputs[i_index].second) {
						is_existing = true;
						break;
					}
				}
				if (!is_existing) {
					input_action_node->input_scope_contexts.push_back(this->inputs[i_index].first.first);
					input_action_node->input_node_context_ids.push_back(this->inputs[i_index].first.second);
					input_action_node->input_obs_indexes.push_back(this->inputs[i_index].second);
				}
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* input_branch_node = (BranchNode*)node;

				bool is_existing = false;
				for (int ii_index = 0; ii_index < (int)input_branch_node->input_scope_contexts.size(); ii_index++) {
					if (input_branch_node->input_scope_contexts[ii_index] == this->inputs[i_index].first.first
							&& input_branch_node->input_node_context_ids[ii_index] == this->inputs[i_index].first.second) {
						is_existing = true;
						break;
					}
				}
				if (!is_existing) {
					input_branch_node->input_scope_contexts.push_back(this->inputs[i_index].first.first);
					input_branch_node->input_node_context_ids.push_back(this->inputs[i_index].first.second);
				}
			}
			break;
		}
	}

	if (this->original_next_node_id == -1) {
		this->original_next_node = NULL;
	} else {
		this->original_next_node = this->parent->nodes[this->original_next_node_id];
	}

	if (this->branch_next_node_id == -1) {
		this->branch_next_node = NULL;
	} else {
		this->branch_next_node = this->parent->nodes[this->branch_next_node_id];
	}
}

void BranchNode::save_for_display(ofstream& output_file) {
	output_file << this->original_next_node_id << endl;
	output_file << this->branch_next_node_id << endl;
}

BranchNodeHistory::BranchNodeHistory(BranchNode* node) {
	this->node = node;
}

BranchNodeHistory::BranchNodeHistory(BranchNodeHistory* original) {
	this->node = original->node;
	this->index = original->index;

	this->is_branch = original->is_branch;
}
