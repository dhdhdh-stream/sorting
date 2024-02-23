#include "retrain_branch_experiment.h"

#include "branch_node.h"
#include "globals.h"
#include "network.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void RetrainBranchExperiment::finalize() {
	if (this->result == EXPERIMENT_RESULT_SUCCESS) {
		this->branch_node->original_average_score = this->original_average_score;
		this->branch_node->branch_average_score = this->branch_average_score;

		this->branch_node->input_scope_context_ids.clear();
		this->branch_node->input_scope_contexts.clear();
		this->branch_node->input_node_context_ids.clear();
		this->branch_node->input_node_contexts.clear();
		vector<int> input_mapping(this->input_scope_contexts.size(), -1);
		for (int i_index = 0; i_index < (int)this->original_linear_weights.size(); i_index++) {
			if (this->original_linear_weights[i_index] != 0.0) {
				if (input_mapping[i_index] == -1) {
					input_mapping[i_index] = (int)this->branch_node->input_scope_contexts.size();
					this->branch_node->input_scope_contexts.push_back(this->input_scope_contexts[i_index]);
					vector<int> scope_context_ids;
					for (int c_index = 0; c_index < (int)this->input_scope_contexts[i_index].size(); c_index++) {
						scope_context_ids.push_back(this->input_scope_contexts[i_index][c_index]->id);
					}
					this->branch_node->input_scope_context_ids.push_back(scope_context_ids);
					this->branch_node->input_node_contexts.push_back(this->input_node_contexts[i_index]);
					vector<int> node_context_ids;
					for (int c_index = 0; c_index < (int)this->input_node_contexts[i_index].size(); c_index++) {
						node_context_ids.push_back(this->input_node_contexts[i_index][c_index]->id);
					}
					this->branch_node->input_node_context_ids.push_back(node_context_ids);
				}
			}
		}
		for (int i_index = 0; i_index < (int)this->original_network_input_indexes.size(); i_index++) {
			for (int v_index = 0; v_index < (int)this->original_network_input_indexes[i_index].size(); v_index++) {
				int original_index = this->original_network_input_indexes[i_index][v_index];
				if (input_mapping[original_index] == -1) {
					input_mapping[original_index] = (int)this->branch_node->input_scope_contexts.size();
					this->branch_node->input_scope_contexts.push_back(this->input_scope_contexts[original_index]);
					vector<int> scope_context_ids;
					for (int c_index = 0; c_index < (int)this->input_scope_contexts[original_index].size(); c_index++) {
						scope_context_ids.push_back(this->input_scope_contexts[original_index][c_index]->id);
					}
					this->branch_node->input_scope_context_ids.push_back(scope_context_ids);
					this->branch_node->input_node_contexts.push_back(this->input_node_contexts[original_index]);
					vector<int> node_context_ids;
					for (int c_index = 0; c_index < (int)this->input_node_contexts[original_index].size(); c_index++) {
						node_context_ids.push_back(this->input_node_contexts[original_index][c_index]->id);
					}
					this->branch_node->input_node_context_ids.push_back(node_context_ids);
				}
			}
		}
		for (int i_index = 0; i_index < (int)this->branch_linear_weights.size(); i_index++) {
			if (this->branch_linear_weights[i_index] != 0.0) {
				if (input_mapping[i_index] == -1) {
					input_mapping[i_index] = (int)this->branch_node->input_scope_contexts.size();
					this->branch_node->input_scope_contexts.push_back(this->input_scope_contexts[i_index]);
					vector<int> scope_context_ids;
					for (int c_index = 0; c_index < (int)this->input_scope_contexts[i_index].size(); c_index++) {
						scope_context_ids.push_back(this->input_scope_contexts[i_index][c_index]->id);
					}
					this->branch_node->input_scope_context_ids.push_back(scope_context_ids);
					this->branch_node->input_node_contexts.push_back(this->input_node_contexts[i_index]);
					vector<int> node_context_ids;
					for (int c_index = 0; c_index < (int)this->input_node_contexts[i_index].size(); c_index++) {
						node_context_ids.push_back(this->input_node_contexts[i_index][c_index]->id);
					}
					this->branch_node->input_node_context_ids.push_back(node_context_ids);
				}
			}
		}
		for (int i_index = 0; i_index < (int)this->branch_network_input_indexes.size(); i_index++) {
			for (int v_index = 0; v_index < (int)this->branch_network_input_indexes[i_index].size(); v_index++) {
				int original_index = this->branch_network_input_indexes[i_index][v_index];
				if (input_mapping[original_index] == -1) {
					input_mapping[original_index] = (int)this->branch_node->input_scope_contexts.size();
					this->branch_node->input_scope_contexts.push_back(this->input_scope_contexts[original_index]);
					vector<int> scope_context_ids;
					for (int c_index = 0; c_index < (int)this->input_scope_contexts[original_index].size(); c_index++) {
						scope_context_ids.push_back(this->input_scope_contexts[original_index][c_index]->id);
					}
					this->branch_node->input_scope_context_ids.push_back(scope_context_ids);
					this->branch_node->input_node_contexts.push_back(this->input_node_contexts[original_index]);
					vector<int> node_context_ids;
					for (int c_index = 0; c_index < (int)this->input_node_contexts[original_index].size(); c_index++) {
						node_context_ids.push_back(this->input_node_contexts[original_index][c_index]->id);
					}
					this->branch_node->input_node_context_ids.push_back(node_context_ids);
				}
			}
		}

		this->branch_node->linear_original_input_indexes.clear();
		this->branch_node->linear_original_weights.clear();
		for (int i_index = 0; i_index < (int)this->original_linear_weights.size(); i_index++) {
			if (this->original_linear_weights[i_index] != 0.0) {
				this->branch_node->linear_original_input_indexes.push_back(input_mapping[i_index]);
				this->branch_node->linear_original_weights.push_back(this->original_linear_weights[i_index]);
			}
		}
		this->branch_node->linear_branch_input_indexes.clear();
		this->branch_node->linear_branch_weights.clear();
		for (int i_index = 0; i_index < (int)this->branch_linear_weights.size(); i_index++) {
			if (this->branch_linear_weights[i_index] != 0.0) {
				this->branch_node->linear_branch_input_indexes.push_back(input_mapping[i_index]);
				this->branch_node->linear_branch_weights.push_back(this->branch_linear_weights[i_index]);
			}
		}

		this->branch_node->original_network_input_indexes.clear();
		for (int i_index = 0; i_index < (int)this->original_network_input_indexes.size(); i_index++) {
			vector<int> input_indexes;
			for (int v_index = 0; v_index < (int)this->original_network_input_indexes[i_index].size(); v_index++) {
				input_indexes.push_back(input_mapping[this->original_network_input_indexes[i_index][v_index]]);
			}
			this->branch_node->original_network_input_indexes.push_back(input_indexes);
		}
		if (this->branch_node->original_network != NULL) {
			delete this->branch_node->original_network;
		}
		this->branch_node->original_network = this->original_network;
		this->original_network = NULL;
		this->branch_node->branch_network_input_indexes.clear();
		for (int i_index = 0; i_index < (int)this->branch_network_input_indexes.size(); i_index++) {
			vector<int> input_indexes;
			for (int v_index = 0; v_index < (int)this->branch_network_input_indexes[i_index].size(); v_index++) {
				input_indexes.push_back(input_mapping[this->branch_network_input_indexes[i_index][v_index]]);
			}
			this->branch_node->branch_network_input_indexes.push_back(input_indexes);
		}
		if (this->branch_node->branch_network != NULL) {
			delete this->branch_node->branch_network;
		}
		this->branch_node->branch_network = this->branch_network;
		this->branch_network = NULL;

		#if defined(MDEBUG) && MDEBUG
		solution->verify_key = this;
		solution->verify_problems = this->verify_problems;
		this->verify_problems.clear();
		solution->verify_seeds = this->verify_seeds;

		this->branch_node->verify_key = this;
		this->branch_node->verify_original_scores = this->verify_original_scores;
		this->branch_node->verify_branch_scores = this->verify_branch_scores;
		#endif /* MDEBUG */
	}

	int experiment_index;
	for (int e_index = 0; e_index < (int)this->branch_node->experiments.size(); e_index++) {
		if (this->branch_node->experiments[e_index] == this) {
			experiment_index = e_index;
		}
	}
	this->branch_node->experiments.erase(this->branch_node->experiments.begin() + experiment_index);
	this->branch_node->experiment_types.erase(this->branch_node->experiment_types.begin() + experiment_index);
}
