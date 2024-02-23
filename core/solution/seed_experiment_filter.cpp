#include "seed_experiment_filter.h"

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "exit_node.h"
#include "globals.h"
#include "network.h"
#include "scope.h"
#include "scope_node.h"
#include "seed_experiment.h"
#include "solution.h"

using namespace std;

SeedExperimentFilter::SeedExperimentFilter(SeedExperiment* parent,
										   vector<Scope*> scope_context,
										   vector<AbstractNode*> node_context,
										   bool is_branch,
										   AbstractNode* seed_next_node,
										   vector<int> filter_step_types,
										   vector<ActionNode*> filter_actions,
										   vector<ScopeNode*> filter_existing_scopes,
										   vector<ScopeNode*> filter_potential_scopes,
										   int filter_exit_depth,
										   AbstractNode* filter_exit_next_node) {
	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;

	this->parent = parent;

	this->seed_next_node = seed_next_node;
	this->filter_step_types = filter_step_types;
	this->filter_actions = filter_actions;
	this->filter_existing_scopes = filter_existing_scopes;
	this->filter_potential_scopes = filter_potential_scopes;
	this->filter_exit_depth = filter_exit_depth;
	this->filter_exit_next_node = filter_exit_next_node;

	this->branch_node = new BranchNode();
	this->branch_node->experiments.push_back(this);
	this->branch_node->experiment_types.push_back(BRANCH_NODE_EXPERIMENT_TYPE_SEED);

	this->network = NULL;

	this->filter_exit_node = NULL;

	this->is_candidate = true;
}

SeedExperimentFilter::~SeedExperimentFilter() {
	if (this->branch_node != NULL) {
		if (this->is_candidate) {
			this->branch_node->experiments.clear();
		}
		delete this->branch_node;
	}

	if (this->network != NULL) {
		delete this->network;
	}

	for (int s_index = 0; s_index < (int)this->filter_actions.size(); s_index++) {
		if (this->filter_actions[s_index] != NULL) {
			delete this->filter_actions[s_index];
		}
	}

	for (int s_index = 0; s_index < (int)this->filter_existing_scopes.size(); s_index++) {
		if (this->filter_existing_scopes[s_index] != NULL) {
			delete this->filter_existing_scopes[s_index];
		}
	}

	for (int s_index = 0; s_index < (int)this->filter_potential_scopes.size(); s_index++) {
		if (this->filter_potential_scopes[s_index] != NULL) {
			delete this->filter_potential_scopes[s_index]->scope;
			delete this->filter_potential_scopes[s_index];
		}
	}

	if (this->filter_exit_node != NULL) {
		delete this->filter_exit_node;
	}
}

void SeedExperimentFilter::add_to_scope() {
	for (int s_index = 0; s_index < (int)this->filter_step_types.size(); s_index++) {
		if (this->filter_step_types[s_index] == STEP_TYPE_ACTION) {
			this->filter_actions[s_index]->parent = this->scope_context.back();
			this->filter_actions[s_index]->id = this->scope_context.back()->node_counter;
			this->scope_context.back()->node_counter++;
		} else if (this->filter_step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
			this->filter_existing_scopes[s_index]->parent = this->scope_context.back();
			this->filter_existing_scopes[s_index]->id = this->scope_context.back()->node_counter;
			this->scope_context.back()->node_counter++;
		} else {
			this->filter_potential_scopes[s_index]->parent = this->scope_context.back();
			this->filter_potential_scopes[s_index]->id = this->scope_context.back()->node_counter;
			this->scope_context.back()->node_counter++;

			int new_scope_id = solution->scope_counter;
			solution->scope_counter++;
			this->filter_potential_scopes[s_index]->scope->id = new_scope_id;

			for (map<int, AbstractNode*>::iterator it = this->filter_potential_scopes[s_index]->scope->nodes.begin();
					it != this->filter_potential_scopes[s_index]->scope->nodes.end(); it++) {
				if (it->second->type == NODE_TYPE_BRANCH) {
					BranchNode* branch_node = (BranchNode*)it->second;
					branch_node->scope_context_ids[0] = new_scope_id;
					for (int i_index = 0; i_index < (int)branch_node->input_scope_context_ids.size(); i_index++) {
						if (branch_node->input_scope_context_ids[i_index].size() > 0) {
							branch_node->input_scope_context_ids[i_index][0] = new_scope_id;
						}
					}
				}
			}
		}
	}

	int end_node_id;
	AbstractNode* end_node;
	if (this->filter_exit_depth > 0) {
		ExitNode* new_exit_node = new ExitNode();
		new_exit_node->parent = this->scope_context.back();
		new_exit_node->id = this->scope_context.back()->node_counter;
		this->scope_context.back()->node_counter++;

		new_exit_node->exit_depth = this->filter_exit_depth;
		new_exit_node->next_node_parent_id = this->scope_context[this->scope_context.size()-1 - this->filter_exit_depth]->id;
		if (this->filter_exit_next_node == NULL) {
			new_exit_node->next_node_id = -1;
		} else {
			new_exit_node->next_node_id = this->filter_exit_next_node->id;
		}
		new_exit_node->next_node = this->filter_exit_next_node;

		this->filter_exit_node = new_exit_node;

		end_node_id = new_exit_node->id;
		end_node = new_exit_node;
	} else {
		if (this->filter_exit_next_node == NULL) {
			end_node_id = -1;
		} else {
			end_node_id = this->filter_exit_next_node->id;
		}
		end_node = this->filter_exit_next_node;
	}

	this->branch_node->experiments.clear();
	this->branch_node->experiment_types.clear();

	this->branch_node->parent = this->scope_context.back();
	this->branch_node->id = this->scope_context.back()->node_counter;
	this->scope_context.back()->node_counter++;

	this->branch_node->scope_context = this->scope_context;
	for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
		this->branch_node->scope_context_ids.push_back(this->branch_node->scope_context[c_index]->id);
	}
	this->branch_node->node_context = this->node_context;
	this->branch_node->node_context.back() = this->branch_node;
	for (int c_index = 0; c_index < (int)this->node_context.size(); c_index++) {
		this->branch_node->node_context_ids.push_back(this->branch_node->node_context[c_index]->id);
	}

	this->branch_node->is_pass_through = false;

	this->branch_node->original_average_score = FILTER_CONFIDENCE_THRESHOLD;
	this->branch_node->branch_average_score = 0.5;

	this->branch_node->input_scope_contexts = this->input_scope_contexts;
	for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
		vector<int> scope_ids;
		for (int v_index = 0; v_index < (int)this->input_scope_contexts[i_index].size(); v_index++) {
			scope_ids.push_back(this->input_scope_contexts[i_index][v_index]->id);
		}
		this->branch_node->input_scope_context_ids.push_back(scope_ids);
	}
	this->branch_node->input_node_contexts = this->input_node_contexts;
	for (int i_index = 0; i_index < (int)this->input_node_contexts.size(); i_index++) {
		vector<int> node_ids;
		for (int v_index = 0; v_index < (int)this->input_node_contexts[i_index].size(); v_index++) {
			node_ids.push_back(this->input_node_contexts[i_index][v_index]->id);
		}
		this->branch_node->input_node_context_ids.push_back(node_ids);
	}

	this->branch_node->original_network = NULL;
	this->branch_node->branch_network_input_indexes = this->network_input_indexes;
	this->branch_node->branch_network = this->network;
	this->network = NULL;

	this->branch_node->original_next_node_id = this->seed_next_node->id;
	this->branch_node->original_next_node = this->seed_next_node;

	if (this->filter_step_types.size() == 0) {
		this->branch_node->branch_next_node_id = end_node_id;
		this->branch_node->branch_next_node = end_node;
	} else {
		if (this->filter_step_types[0] == STEP_TYPE_ACTION) {
			this->branch_node->branch_next_node_id = this->filter_actions[0]->id;
			this->branch_node->branch_next_node = this->filter_actions[0];
		} else if (this->filter_step_types[0] == STEP_TYPE_EXISTING_SCOPE) {
			this->branch_node->branch_next_node_id = this->filter_existing_scopes[0]->id;
			this->branch_node->branch_next_node = this->filter_existing_scopes[0];
		} else {
			this->branch_node->branch_next_node_id = this->filter_potential_scopes[0]->id;
			this->branch_node->branch_next_node = this->filter_potential_scopes[0];
		}
	}

	for (int s_index = 0; s_index < (int)this->filter_step_types.size(); s_index++) {
		int next_node_id;
		AbstractNode* next_node;
		if (s_index == (int)this->filter_step_types.size()-1) {
			next_node_id = end_node_id;
			next_node = end_node;
		} else {
			if (this->filter_step_types[s_index+1] == STEP_TYPE_ACTION) {
				next_node_id = this->filter_actions[s_index+1]->id;
				next_node = this->filter_actions[s_index+1];
			} else if (this->filter_step_types[s_index+1] == STEP_TYPE_EXISTING_SCOPE) {
				next_node_id = this->filter_existing_scopes[s_index+1]->id;
				next_node = this->filter_existing_scopes[s_index+1];
			} else {
				next_node_id = this->filter_potential_scopes[s_index+1]->id;
				next_node = this->filter_potential_scopes[s_index+1];
			}
		}

		if (this->filter_step_types[s_index] == STEP_TYPE_ACTION) {
			this->filter_actions[s_index]->next_node_id = next_node_id;
			this->filter_actions[s_index]->next_node = next_node;
		} else if (this->filter_step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
			this->filter_existing_scopes[s_index]->next_node_id = next_node_id;
			this->filter_existing_scopes[s_index]->next_node = next_node;
		} else {
			this->filter_potential_scopes[s_index]->next_node_id = next_node_id;
			this->filter_potential_scopes[s_index]->next_node = next_node;
		}
	}

	this->is_candidate = false;
}

void SeedExperimentFilter::finalize() {
	if (this->node_context.back()->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)this->node_context.back();

		action_node->next_node_id = this->branch_node->id;
		action_node->next_node = this->branch_node;
	} else if (this->node_context.back()->type == NODE_TYPE_SCOPE) {
		ScopeNode* scope_node = (ScopeNode*)this->node_context.back();

		scope_node->next_node_id = this->branch_node->id;
		scope_node->next_node = this->branch_node;
	} else {
		BranchNode* branch_node = (BranchNode*)this->node_context.back();

		if (this->is_branch) {
			branch_node->branch_next_node_id = this->branch_node->id;
			branch_node->branch_next_node = this->branch_node;
		} else {
			branch_node->original_next_node_id = this->branch_node->id;
			branch_node->original_next_node = this->branch_node;
		}
	}

	this->scope_context.back()->nodes[this->branch_node->id] = this->branch_node;
	this->branch_node = NULL;

	for (int s_index = 0; s_index < (int)this->filter_step_types.size(); s_index++) {
		if (this->filter_step_types[s_index] == STEP_TYPE_ACTION) {
			this->scope_context.back()->nodes[this->filter_actions[s_index]->id] = this->filter_actions[s_index];
		} else if (this->filter_step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
			this->scope_context.back()->nodes[this->filter_existing_scopes[s_index]->id] = this->filter_existing_scopes[s_index];
		} else {
			this->scope_context.back()->nodes[this->filter_potential_scopes[s_index]->id] = this->filter_potential_scopes[s_index];

			solution->scopes[this->filter_potential_scopes[s_index]->scope->id] = this->filter_potential_scopes[s_index]->scope;
		}
	}
	this->filter_actions.clear();
	this->filter_existing_scopes.clear();
	this->filter_potential_scopes.clear();

	if (this->filter_exit_node != NULL) {
		this->scope_context.back()->nodes[this->filter_exit_node->id] = this->filter_exit_node;
	}
	this->filter_exit_node = NULL;

	// let success_reset() clean
}

void SeedExperimentFilter::backprop(double target_val,
									RunHelper& run_helper,
									AbstractExperimentHistory* history) {
	// do nothing
}
