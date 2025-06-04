#include "branch_experiment.h"

#include <iostream>

#include "abstract_node.h"
#include "globals.h"
#include "problem.h"

using namespace std;

BranchExperiment::BranchExperiment(Scope* scope_context,
								   AbstractNode* node_context,
								   bool is_branch) {
	this->type = EXPERIMENT_TYPE_BRANCH;

	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;

	this->best_exit_next_node = NULL;

	this->sum_num_instances = 0;

	this->state = BRANCH_EXPERIMENT_STATE_EXISTING_GATHER;
	this->state_iter = 0;

	this->result = EXPERIMENT_RESULT_NA;
}

BranchExperiment::~BranchExperiment() {
	this->node_context->experiment = NULL;
}

void BranchExperiment::decrement(AbstractNode* experiment_node) {
	delete this;
}

void BranchExperiment::clean_inputs(Scope* scope,
									int node_id) {
	for (int i_index = (int)this->existing_inputs.size()-1; i_index >= 0; i_index--) {
		bool is_match = false;
		for (int l_index = 0; l_index < (int)this->existing_inputs[i_index].scope_context.size(); l_index++) {
			if (this->existing_inputs[i_index].scope_context[l_index] == scope
					&& this->existing_inputs[i_index].node_context[l_index] == node_id) {
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

				for (int h_index = 0; h_index < (int)this->existing_factor_histories.size(); h_index++) {
					this->existing_factor_histories[h_index].erase(this->existing_factor_histories[h_index].begin() + f_index);
				}

				if (this->existing_factor_weights.size() > 0) {
					this->existing_factor_weights.erase(this->existing_factor_weights.begin() + f_index);
				}
			}
		}
	}

	if (scope == this->scope_context) {
		for (int e_index = 0; e_index < (int)this->explore_exit_next_nodes.size(); e_index++) {
			if (this->explore_exit_next_nodes[e_index] != NULL) {
				if (this->explore_exit_next_nodes[e_index]->id == node_id) {
					this->explore_input_histories.erase(this->explore_input_histories.begin() + e_index);
					this->explore_factor_histories.erase(this->explore_factor_histories.begin() + e_index);
					this->explore_step_types.erase(this->explore_step_types.begin() + e_index);
					this->explore_actions.erase(this->explore_actions.begin() + e_index);
					this->explore_scopes.erase(this->explore_scopes.begin() + e_index);
					this->explore_exit_next_nodes.erase(this->explore_exit_next_nodes.begin() + e_index);
					this->explore_target_val_histories.erase(this->explore_target_val_histories.begin() + e_index);
				}
			}
		}
	}

	if (scope == this->scope_context) {
		if (this->best_exit_next_node != NULL) {
			if (this->best_exit_next_node->id == node_id) {
				delete this;
				return;
			}
		}
	}

	for (int i_index = (int)this->new_inputs.size()-1; i_index >= 0; i_index--) {
		bool is_match = false;
		for (int l_index = 0; l_index < (int)this->new_inputs[i_index].scope_context.size(); l_index++) {
			if (this->new_inputs[i_index].scope_context[l_index] == scope
					&& this->new_inputs[i_index].node_context[l_index] == node_id) {
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

				if (this->new_factor_weights.size() > 0) {
					this->new_factor_weights.erase(this->new_factor_weights.begin() + f_index);
				}
			}
		}
	}
}

void BranchExperiment::clean_inputs(Scope* scope) {
	for (int i_index = (int)this->existing_inputs.size()-1; i_index >= 0; i_index--) {
		bool is_match = false;
		for (int l_index = 0; l_index < (int)this->existing_inputs[i_index].scope_context.size(); l_index++) {
			if (this->existing_inputs[i_index].scope_context[l_index] == scope) {
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
		for (int l_index = 0; l_index < (int)this->new_inputs[i_index].scope_context.size(); l_index++) {
			if (this->new_inputs[i_index].scope_context[l_index] == scope) {
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
}

void BranchExperiment::replace_factor(Scope* scope,
									  int original_node_id,
									  int original_factor_index,
									  int new_node_id,
									  int new_factor_index) {
	for (int i_index = 0; i_index < (int)this->existing_inputs.size(); i_index++) {
		if (this->existing_inputs[i_index].scope_context.back() == scope
				&& this->existing_inputs[i_index].node_context.back() == original_node_id
				&& this->existing_inputs[i_index].factor_index == original_factor_index) {
			this->existing_inputs[i_index].node_context.back() = new_node_id;
			this->existing_inputs[i_index].factor_index = new_factor_index;
		}
	}

	if (scope == this->scope_context) {
		for (int f_index = 0; f_index < (int)this->existing_factor_ids.size(); f_index++) {
			if (this->existing_factor_ids[f_index].first == original_node_id
					&& this->existing_factor_ids[f_index].second == original_factor_index) {
				this->existing_factor_ids[f_index].first = new_node_id;
				this->existing_factor_ids[f_index].second = new_factor_index;
			}
		}
	}

	for (int i_index = 0; i_index < (int)this->new_inputs.size(); i_index++) {
		if (this->new_inputs[i_index].scope_context.back() == scope
				&& this->new_inputs[i_index].node_context.back() == original_node_id
				&& this->new_inputs[i_index].factor_index == original_factor_index) {
			this->new_inputs[i_index].node_context.back() = new_node_id;
			this->new_inputs[i_index].factor_index = new_factor_index;
		}
	}

	if (scope == this->scope_context) {
		for (int f_index = 0; f_index < (int)this->new_factor_ids.size(); f_index++) {
			if (this->new_factor_ids[f_index].first == original_node_id
					&& this->new_factor_ids[f_index].second == original_factor_index) {
				this->new_factor_ids[f_index].first = new_node_id;
				this->new_factor_ids[f_index].second = new_factor_index;
			}
		}
	}
}

void BranchExperiment::replace_obs_node(Scope* scope,
										int original_node_id,
										int new_node_id) {
	for (int i_index = 0; i_index < (int)this->existing_inputs.size(); i_index++) {
		if (this->existing_inputs[i_index].scope_context.back() == scope
				&& this->existing_inputs[i_index].node_context.back() == original_node_id) {
			this->existing_inputs[i_index].node_context.back() = new_node_id;
		}
	}

	for (int i_index = 0; i_index < (int)this->new_inputs.size(); i_index++) {
		if (this->new_inputs[i_index].scope_context.back() == scope
				&& this->new_inputs[i_index].node_context.back() == original_node_id) {
			this->new_inputs[i_index].node_context.back() = new_node_id;
		}
	}
}

void BranchExperiment::replace_scope(Scope* original_scope,
									 Scope* new_scope,
									 int new_scope_node_id) {
	for (int i_index = 0; i_index < (int)this->existing_inputs.size(); i_index++) {
		for (int l_index = 1; l_index < (int)this->existing_inputs[i_index].scope_context.size(); l_index++) {
			if (this->existing_inputs[i_index].scope_context[l_index] == original_scope) {
				this->existing_inputs[i_index].scope_context.insert(
					this->existing_inputs[i_index].scope_context.begin() + l_index, new_scope);
				this->existing_inputs[i_index].node_context.insert(
					this->existing_inputs[i_index].node_context.begin() + l_index, new_scope_node_id);
				break;
			}
		}
	}

	for (int e_index = 0; e_index < (int)this->explore_scopes.size(); e_index++) {
		for (int a_index = 0; a_index < (int)this->explore_scopes[e_index].size(); a_index++) {
			if (this->explore_scopes[e_index][a_index] == original_scope) {
				this->explore_scopes[e_index][a_index] = new_scope;
			}
		}
	}

	for (int a_index = 0; a_index < (int)this->best_scopes.size(); a_index++) {
		if (this->best_scopes[a_index] == original_scope) {
			this->best_scopes[a_index] = new_scope;
		}
	}

	for (int i_index = 0; i_index < (int)this->new_inputs.size(); i_index++) {
		for (int l_index = 1; l_index < (int)this->new_inputs[i_index].scope_context.size(); l_index++) {
			if (this->new_inputs[i_index].scope_context[l_index] == original_scope) {
				this->new_inputs[i_index].scope_context.insert(
					this->new_inputs[i_index].scope_context.begin() + l_index, new_scope);
				this->new_inputs[i_index].node_context.insert(
					this->new_inputs[i_index].node_context.begin() + l_index, new_scope_node_id);
				break;
			}
		}
	}
}

BranchExperimentHistory::BranchExperimentHistory(BranchExperiment* experiment) {
	this->experiment = experiment;

	switch (experiment->state) {
	case BRANCH_EXPERIMENT_STATE_EXISTING_GATHER:
	case BRANCH_EXPERIMENT_STATE_NEW_GATHER:
		this->is_active = false;
		break;
	case BRANCH_EXPERIMENT_STATE_EXPLORE:
	case BRANCH_EXPERIMENT_STATE_TRAIN_NEW:
	case BRANCH_EXPERIMENT_STATE_MEASURE_1_PERCENT:
		{
			uniform_int_distribution<int> active_distribution(0, 99);
			if (active_distribution(generator) == 0) {
				this->is_active = true;
			} else {
				this->is_active = false;
			}
		}
		break;
	case BRANCH_EXPERIMENT_STATE_MEASURE_5_PERCENT:
		{
			uniform_int_distribution<int> active_distribution(0, 19);
			if (active_distribution(generator) == 0) {
				this->is_active = true;
			} else {
				this->is_active = false;
			}
		}
		break;
	case BRANCH_EXPERIMENT_STATE_MEASURE_10_PERCENT:
		{
			uniform_int_distribution<int> active_distribution(0, 9);
			if (active_distribution(generator) == 0) {
				this->is_active = true;
			} else {
				this->is_active = false;
			}
		}
		break;
	case BRANCH_EXPERIMENT_STATE_MEASURE_25_PERCENT:
		{
			uniform_int_distribution<int> active_distribution(0, 3);
			if (active_distribution(generator) == 0) {
				this->is_active = true;
			} else {
				this->is_active = false;
			}
		}
		break;
	case BRANCH_EXPERIMENT_STATE_MEASURE_50_PERCENT:
		{
			uniform_int_distribution<int> active_distribution(0, 1);
			if (active_distribution(generator) == 0) {
				this->is_active = true;
			} else {
				this->is_active = false;
			}
		}
		break;
	}
}
