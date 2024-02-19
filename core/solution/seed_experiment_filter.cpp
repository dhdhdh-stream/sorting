#include "seed_experiment_filter.h"

using namespace std;

SeedExperimentFilter::SeedExperimentFilter(SeedExperiment* parent,
										   vector<Scope*> scope_context,
										   vector<AbstractNode*> node_context,
										   bool is_branch,
										   vector<int> step_types,
										   vector<ActionNode*> actions,
										   vector<ScopeNode*> existing_scopes,
										   vector<ScopeNode*> potential_scopes,
										   int exit_depth,
										   AbstractNode* exit_next_node) {
	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;

	this->parent = parent;

	this->step_types = step_types;
	this->actions = actions;
	this->existing_scopes = existing_scopes;
	this->potential_scopes = potential_scopes;
	this->exit_depth = exit_depth;
	this->exit_next_node = exit_next_node;

	this->network = NULL;

	this->exit_node = NULL;

	this->is_candidate = false;
}

SeedExperimentFilter::~SeedExperimentFilter() {
	if (this->network != NULL) {
		delete this->network;
	}
	/**
	 * - let local scope delete nodes
	 */
}

void SeedExperimentFilter::finalize_success() {
	BranchNode* new_branch_node = new BranchNode();
	new_branch_node->parent = this->scope_context.back();
	new_branch_node->id = this->scope_context.back()->node_counter;
	this->scope_context.back()->node_counter++;
	this->scope_context.back()->nodes[new_branch_node->id] = new_branch_node;

	new_branch_node->scope_context = this->scope_context;
	for (int c_index = 0; c_index < (int)new_branch_node->scope_context.size(); c_index++) {
		new_branch_node->scope_context_ids.push_back(new_branch_node->scope_context[c_index]->id);
	}
	new_branch_node->node_context = this->node_context;
	new_branch_node->node_context.back() = new_branch_node;
	for (int c_index = 0; c_index < (int)new_branch_node->node_context.size(); c_index++) {
		new_branch_node->node_context_ids.push_back(new_branch_node->node_context[c_index]->id);
	}

	new_branch_node->is_pass_through = false;

	new_branch_node->original_average_score = FILTER_CONFIDENCE_THRESHOLD;
	new_branch_node->branch_average_score = this->average_confidence;

	for (int l_index = 0; l_index < (int)this->linear_weights.size(); l_index++) {
		new_branch_node->linear_branch_input_indexes.push_back(l_index);
	}
	new_branch_node->linear_branch_weights = this->linear_weights;

	new_branch_node->original_network = NULL;
	new_branch_node->branch_network_input_indexes = this->network_input_indexes;
	new_branch_node->branch_network = this->network;
	this->network = NULL;

	if (this->node_context.back()->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)this->node_context.back();

		new_branch_node->original_next_node_id = action_node->next_node_id;
		new_branch_node->original_next_node = action_node->next_node;
	} else if (this->node_context.back()->type == NODE_TYPE_SCOPE) {
		ScopeNode* scope_node = (ScopeNode*)this->node_context.back();

		new_branch_node->original_next_node_id = scope_node->next_node_id;
		new_branch_node->original_next_node = scope_node->next_node;
	} else {
		BranchNode* branch_node = (BranchNode*)this->node_context.back();

		if (this->is_branch) {
			new_branch_node->original_next_node_id = branch_node->branch_next_node_id;
			new_branch_node->original_next_node = branch_node->branch_next_node;
		} else {
			new_branch_node->original_next_node_id = branch_node->original_next_node_id;
			new_branch_node->original_next_node = branch_node->original_next_node;
		}
	}

	if (this->step_types.size() == 0) {
		new_branch_node->branch_next_node_id = this->exit_node->id;
		new_branch_node->branch_next_node = this->exit_node;
	} else {
		if (this->step_types[0] == STEP_TYPE_ACTION) {
			new_branch_node->branch_next_node_id = this->actions[0]->id;
			new_branch_node->branch_next_node = this->actions[0];
		} else if (this->step_types[0] == STEP_TYPE_EXISTING_SCOPE) {
			new_branch_node->branch_next_node_id = this->existing_scopes[0]->id;
			new_branch_node->branch_next_node = this->existing_scopes[0];
		} else {
			new_branch_node->branch_next_node_id = this->potential_scopes[0]->id;
			new_branch_node->branch_next_node = this->potential_scopes[0];
		}
	}

	if (this->node_context.back()->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)this->node_context.back();

		action_node->next_node_id = new_branch_node->id;
		action_node->next_node = new_branch_node;
	} else if (this->node_context.back()->type == NODE_TYPE_SCOPE) {
		ScopeNode* scope_node = (ScopeNode*)this->node_context.back();

		scope_node->next_node_id = new_branch_node->id;
		scope_node->next_node = new_branch_node;
	} else {
		BranchNode* branch_node = (BranchNode*)this->node_context.back();

		if (this->is_branch) {
			branch_node->branch_next_node_id = new_branch_node->id;
			branch_node->branch_next_node = new_branch_node;
		} else {
			branch_node->original_next_node_id = new_branch_node->id;
			branch_node->original_next_node = new_branch_node;
		}
	}

	for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
		if (this->step_types[s_index] == STEP_TYPE_POTENTIAL_SCOPE) {
			solution->scopes[this->potential_scopes[s_index]->scope->id] = this->potential_scopes[s_index]->scope;
		}
	}

	// let success_reset() clean
}

void SeedExperimentFilter::clean_fail() {
	for (int a_index = 0; a_index < (int)this->step_types.size(); a_index++) {
		if (this->step_types[a_index] == STEP_TYPE_ACTION) {
			this->scope_context.back()->nodes.remove(this->actions[a_index]->id);
			delete this->actions[a_index];
		} else if (this->step_types[a_index] == STEP_TYPE_EXISTING_SCOPE) {
			this->scope_context.back()->nodes.remove(this->existing_scopes[a_index]->id);
			delete this->existing_scopes[a_index];
		} else {
			this->scope_context.back()->nodes.remove(this->potential_scopes[a_index]->id);
			delete this->potential_scopes[a_index]->scope;
			delete this->potential_scopes[a_index];
		}
	}

	if (this->exit_node != NULL) {
		this->scope_context.back()->nodes.remove(this->exit_node->id);
		delete this->exit_node;
	}

	if (this->node_context.back()->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)this->node_context.back();
		int experiment_index;
		for (int e_index = 0; e_index < (int)action_node->experiments.size(); e_index++) {
			if (action_node->experiments[e_index] == this) {
				experiment_index = e_index;
			}
		}
		action_node->experiments.erase(action_node->experiments.begin() + e_index);
	} else if (this->node_context.back()->type == NODE_TYPE_SCOPE) {
		ScopeNode* scope_node = (ScopeNode*)this->node_context.back();
		int experiment_index;
		for (int e_index = 0; e_index < (int)scope_node->experiments.size(); e_index++) {
			if (scope_node->experiments[e_index] == this) {
				experiment_index = e_index;
			}
		}
		scope_node->experiments.erase(scope_node->experiments.begin() + e_index);
	} else {
		BranchNode* branch_node = (BranchNode*)this->node_context.back();
		int experiment_index;
		for (int e_index = 0; e_index < (int)branch_node->experiments.size(); e_index++) {
			if (branch_node->experiments[e_index] == this) {
				experiment_index = e_index;
			}
		}
		branch_node->experiments.erase(branch_node->experiments.begin() + e_index);
		branch_node->experiment_is_branch.erase(branch_node->experiment_is_branch.begin() + e_index);
	}
	// delete in this->parent
}
