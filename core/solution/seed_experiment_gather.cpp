#include "seed_experiment_gather.h"

using namespace std;

SeedExperimentGather::SeedExperimentGather(SeedExperiment* parent,
										   vector<Scope*> scope_context,
										   vector<AbstractNode*> node_context,
										   bool is_branch,
										   vector<int> step_types,
										   vector<ActionNode*> actions,
										   vector<ScopeNode*> existing_scopes,
										   vector<ScopeNode*> potential_scopes,
										   int exit_depth,
										   AbstractNode* exit_node) {
	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;

	this->parent = parent;

	this->step_types = step_types;
	this->actions = actions;
	this->existing_scopes = existing_scopes;
	this->potential_scopes = potential_scopes;

	int end_node_id;
	AbstractNode* end_node;
	if (exit_depth > 0) {
		ExitNode* new_exit_node = new ExitNode();
		new_exit_node->parent = this->scope_context.back();
		new_exit_node->id = this->scope_context.back()->node_counter;
		this->scope_context.back()->node_counter++;
		this->scope_context.back()->nodes[new_exit_node->id] = new_exit_node;

		new_exit_node->exit_depth = exit_depth;
		new_exit_node->exit_node_parent_id = this->scope_context[this->scope_context.size()-1 - exit_depth]->id;
		if (exit_node == NULL) {
			new_exit_node->exit_node_id = -1;
		} else {
			new_exit_node->exit_node_id = exit_node->id;
		}
		new_exit_node->exit_node = exit_node;

		this->exit_node = new_exit_node;

		end_node_id = new_exit_node->id;
		end_node = new_exit_node;
	} else {
		this->exit_node = NULL;

		if (exit_node == NULL) {
			end_node_id = -1;
		} else {
			end_node_id = exit_node->id;
		}
		end_node = exit_node;
	}

	for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
		int next_node_id;
		AbstractNode* next_node;
		if (s_index == (int)this->step_types.size()-1) {
			next_node_id = end_node_id;
			new_exit_node = end_node;
		} else {
			if (this->step_types[s_index+1] == STEP_TYPE_ACTION) {
				next_node_id = this->actions[s_index+1]->id;
				next_node = this->actions[s_index+1];
			} else if (this->step_types[s_index+1] == STEP_TYPE_EXISTING_SCOPE) {
				next_node_id = this->existing_scopes[s_index+1]->id;
				next_node = this->existing_scopes[s_index+1];
			} else {
				next_node_id = this->potential_scopes[s_index+1]->id;
				next_node = this->potential_scopes[s_index+1];
			}
		}

		if (this->step_types[s_index] == STEP_TYPE_ACTION) {
			this->scope_context.back()->nodes[this->actions[s_index]->id] = this->actions[s_index];

			this->actions[s_index]->next_node_id = next_node_id;
			this->actions[s_index]->next_node = next_node;
		} else if (this->step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
			this->scope_context.back()->nodes[this->existing_scopes[s_index]->id] = this->existing_scopes[s_index];

			this->existing_scopes[s_index]->next_node_id = next_node_id;
			this->existing_scopes[s_index]->next_node = next_node;
		} else {
			this->scope_context.back()->nodes[this->potential_scopes[s_index]->id] = this->potential_scopes[s_index];

			this->potential_scopes[s_index]->next_node_id = next_node_id;
			this->potential_scopes[s_index]->next_node = next_node;
		}
	}
}

SeedExperimentGather::~SeedExperimentGather() {
	// do nothing
}

void SeedExperimentGather::activate(AbstractNode*& curr_node,
									vector<ContextLayer>& context,
									RunHelper& run_helper) {
	bool is_selected = false;
	if (run_helper.experiment_history == NULL) {
		bool matches_context = true;
		if (this->scope_context.size() > context.size()) {
			matches_context = false;
		} else {
			for (int c_index = 0; c_index < (int)this->scope_context.size()-1; c_index++) {
				if (this->scope_context[c_index] != context[context.size()-this->scope_context.size()+c_index].scope
						|| this->node_context[c_index] != context[context.size()-this->scope_context.size()+c_index].node) {
					matches_context = false;
					break;
				}
			}
		}

		if (matches_context) {
			bool has_seen = false;
			for (int e_index = 0; e_index < (int)run_helper.experiments_seen_order.size(); e_index++) {
				if (run_helper.experiments_seen_order[e_index] == this->parent) {
					has_seen = true;
					break;
				}
			}
			if (!has_seen) {
				double selected_probability = 1.0 / (1.0 + this->parent->average_remaining_experiments_from_start);
				uniform_real_distribution<double> distribution(0.0, 1.0);
				if (distribution(generator) < selected_probability) {
					run_helper.experiment_history = new SeedExperimentOverallHistory(this->parent);
					is_selected = true;
				}

				run_helper.experiments_seen_order.push_back(this->parent);
			}
		}
	} else if (run_helper.experiment_history->experiment == this->parent) {
		bool matches_context = true;
		if (this->scope_context.size() > context.size()) {
			matches_context = false;
		} else {
			for (int c_index = 0; c_index < (int)this->scope_context.size()-1; c_index++) {
				if (this->scope_context[c_index] != context[context.size()-this->scope_context.size()+c_index].scope
						|| this->node_context[c_index] != context[context.size()-this->scope_context.size()+c_index].node) {
					matches_context = false;
					break;
				}
			}
		}

		if (matches_context) {
			is_selected = true;
		}
	}

	if (is_selected) {
		if (this->step_types[0] == STEP_TYPE_ACTION) {
			curr_node = this->actions[0];
		} else if (this->step_types[0] == STEP_TYPE_EXISTING_SCOPE) {
			curr_node = this->existing_scopes[0];
		} else {
			curr_node = this->potential_scopes[0];
		}
	}
}

void SeedExperimentGather::finalize_success() {
	int start_node_id;
	AbstractNode* start_node;
	if (this->scope_context.size() > 0) {
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

		new_branch_node->is_pass_through = true;

		new_branch_node->original_average_score = 0.0;
		new_branch_node->branch_average_score = 0.0;

		new_branch_node->original_network = NULL;
		new_branch_node->branch_network = NULL;

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

		start_node_id = new_branch_node->id;
		start_node = new_branch_node;
	} else {
		if (this->step_types[0] == STEP_TYPE_ACTION) {
			start_node_id = this->actions[0]->id;
			start_node = this->actions[0];
		} else if (this->step_types[0] == STEP_TYPE_EXISTING_SCOPE) {
			start_node_id = this->existing_scopes[0]->id;
			start_node = this->existing_scopes[0];
		} else {
			start_node_id = this->potential_scopes[0]->id;
			start_node = this->potential_scopes[0];
		}
	}

	if (this->node_context.back()->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)this->node_context.back();

		action_node->next_node_id = start_node_id;
		action_node->next_node = start_node;
	} else if (this->node_context.back()->type == NODE_TYPE_SCOPE) {
		ScopeNode* scope_node = (ScopeNode*)this->node_context.back();

		scope_node->next_node_id = start_node_id;
		scope_node->next_node = start_node;
	} else {
		BranchNode* branch_node = (BranchNode*)this->node_context.back();

		if (this->is_branch) {
			branch_node->branch_next_node_id = start_node_id;
			branch_node->branch_next_node = start_node;
		} else {
			branch_node->original_next_node_id = start_node_id;
			branch_node->original_next_node = start_node;
		}
	}

	for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
		if (this->step_types[s_index] == STEP_TYPE_POTENTIAL_SCOPE) {
			solution->scopes[this->potential_scopes[s_index]->scope->id] = this->potential_scopes[s_index]->scope;
		}
	}

	// let success_reset() clean
}

void SeedExperimentGather::clean_fail() {
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
