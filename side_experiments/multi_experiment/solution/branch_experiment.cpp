#include "branch_experiment.h"

#include <iostream>

using namespace std;

BranchExperiment::BranchExperiment(Scope* scope_context,
								   AbstractNode* node_context,
								   bool is_branch) {
	this->type = EXPERIMENT_TYPE_BRANCH;

	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;

	this->parent_experiment = NULL;

	this->state = BRANCH_EXPERIMENT_STATE_EXISTING_GATHER;
	this->state_iter = 0;

	this->result = EXPERIMENT_RESULT_NA;
}

void BranchExperiment::decrement(AbstractNode* experiment_node) {
	delete this;
}

void BranchExperiment::clean_inputs(Scope* scope,
									int node_id) {
	for (int i_index = (int)this->existing_inputs.size()-1; i_index >= 0; i_index--) {
		bool is_match = false;
		for (int l_index = 0; l_index < (int)this->existing_inputs[i_index].first.first.size(); l_index++) {
			if (this->existing_inputs[i_index].first.first[l_index] == scope
					&& this->existing_inputs[i_index].first.second[l_index] == node_id) {
				is_match = true;
				break;
			}
		}

		if (is_match) {
			this->existing_inputs.erase(this->existing_inputs.begin() + i_index);
			for (int h_index = 0; h_index < (int)this->existing_input_histories.size(); h_index++) {
				this->existing_input_histories[h_index].erase(this->existing_input_histories[h_index].begin() + i_index);
			}
		}
	}

	if (scope == this->scope_context) {
		for (int f_index = (int)this->existing_factor_ids.size()-1; f_index >= 0; f_index--) {
			if (this->existing_factor_ids[f_index].first == node_id) {
				this->existing_factor_ids.erase(this->existing_factor_ids.begin() + f_index);
				if (this->existing_factor_weights.size() > 0) {
					this->existing_factor_weights.erase(this->existing_factor_weights.begin() + f_index);
				}
				for (int h_index = 0; h_index < (int)this->existing_factor_histories.size(); h_index++) {
					this->existing_factor_histories[h_index].erase(this->existing_factor_histories[h_index].begin() + f_index);
				}
			}
		}
	}

	for (int i_index = (int)this->new_inputs.size()-1; i_index >= 0; i_index--) {
		bool is_match = false;
		for (int l_index = 0; l_index < (int)this->new_inputs[i_index].first.first.size(); l_index++) {
			if (this->new_inputs[i_index].first.first[l_index] == scope
					&& this->new_inputs[i_index].first.second[l_index] == node_id) {
				is_match = true;
				break;
			}
		}

		if (is_match) {
			this->new_inputs.erase(this->new_inputs.begin() + i_index);
			for (int h_index = 0; h_index < (int)this->new_input_histories.size(); h_index++) {
				this->new_input_histories[h_index].erase(this->new_input_histories[h_index].begin() + i_index);
			}
		}
	}

	if (scope == this->scope_context) {
		for (int f_index = (int)this->new_factor_ids.size()-1; f_index >= 0; f_index--) {
			if (this->new_factor_ids[f_index].first == node_id) {
				this->new_factor_ids.erase(this->new_factor_ids.begin() + f_index);
				for (int h_index = 0; h_index < (int)this->new_factor_histories.size(); h_index++) {
					this->new_factor_histories[h_index].erase(this->new_factor_histories[h_index].begin() + f_index);
				}
			}
		}
	}
}

void BranchExperiment::clean_inputs(Scope* scope) {
	for (int i_index = (int)this->existing_inputs.size()-1; i_index >= 0; i_index--) {
		bool is_match = false;
		for (int l_index = 0; l_index < (int)this->existing_inputs[i_index].first.first.size(); l_index++) {
			if (this->existing_inputs[i_index].first.first[l_index] == scope) {
				is_match = true;
				break;
			}
		}

		if (is_match) {
			this->existing_inputs.erase(this->existing_inputs.begin() + i_index);
			for (int h_index = 0; h_index < (int)this->existing_input_histories.size(); h_index++) {
				this->existing_input_histories[h_index].erase(this->existing_input_histories[h_index].begin() + i_index);
			}
		}
	}

	for (int i_index = (int)this->new_inputs.size()-1; i_index >= 0; i_index--) {
		bool is_match = false;
		for (int l_index = 0; l_index < (int)this->new_inputs[i_index].first.first.size(); l_index++) {
			if (this->new_inputs[i_index].first.first[l_index] == scope) {
				is_match = true;
				break;
			}
		}

		if (is_match) {
			this->new_inputs.erase(this->new_inputs.begin() + i_index);
			for (int h_index = 0; h_index < (int)this->new_inputs.size(); h_index++) {
				this->new_input_histories[h_index].erase(this->new_input_histories[h_index].begin() + i_index);
			}
		}
	}
}

BranchExperimentHistory::BranchExperimentHistory(BranchExperiment* experiment) {
	this->experiment = experiment;

	this->impact = 0.0;
}
