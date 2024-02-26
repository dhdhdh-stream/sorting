#include "seed_experiment_gather.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "exit_node.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"
#include "seed_experiment.h"
#include "solution.h"

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
										   AbstractNode* exit_next_node) {
	this->type = EXPERIMENT_TYPE_SEED_GATHER;

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

	this->exit_node = NULL;

	this->is_candidate = true;
}

SeedExperimentGather::~SeedExperimentGather() {
	for (int s_index = 0; s_index < (int)this->actions.size(); s_index++) {
		if (this->actions[s_index] != NULL) {
			delete this->actions[s_index];
		}
	}

	for (int s_index = 0; s_index < (int)this->existing_scopes.size(); s_index++) {
		if (this->existing_scopes[s_index] != NULL) {
			delete this->existing_scopes[s_index];
		}
	}

	for (int s_index = 0; s_index < (int)this->potential_scopes.size(); s_index++) {
		if (this->potential_scopes[s_index] != NULL) {
			delete this->potential_scopes[s_index]->scope;
			delete this->potential_scopes[s_index];
		}
	}

	if (this->exit_node != NULL) {
		delete this->exit_node;
	}
}

bool SeedExperimentGather::activate(AbstractNode*& curr_node,
									Problem* problem,
									vector<ContextLayer>& context,
									int& exit_depth,
									AbstractNode*& exit_node,
									RunHelper& run_helper,
									AbstractExperimentHistory*& history) {
	bool is_selected = false;
	if (this->parent->state != SEED_EXPERIMENT_STATE_VERIFY_1ST_EXISTING
			&& this->parent->state != SEED_EXPERIMENT_STATE_VERIFY_2ND_EXISTING) {
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
	}

	if (is_selected) {
		if (this->is_candidate) {
			for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
				if (this->step_types[s_index] == STEP_TYPE_ACTION) {
					ActionNodeHistory* action_node_history = new ActionNodeHistory(this->actions[s_index]);
					this->actions[s_index]->activate(curr_node,
													 problem,
													 context,
													 exit_depth,
													 exit_node,
													 run_helper,
													 action_node_history);
					delete action_node_history;
				} else if (this->step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
					ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(this->existing_scopes[s_index]);
					this->existing_scopes[s_index]->activate(curr_node,
															 problem,
															 context,
															 exit_depth,
															 exit_node,
															 run_helper,
															 scope_node_history);
					delete scope_node_history;
				} else {
					ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(this->potential_scopes[s_index]);
					this->potential_scopes[s_index]->activate(curr_node,
															  problem,
															  context,
															  exit_depth,
															  exit_node,
															  run_helper,
															  scope_node_history);
					delete scope_node_history;
				}
			}

			if (this->exit_depth == 0) {
				curr_node = this->exit_next_node;
			} else {
				exit_depth = this->exit_depth-1;
				exit_node = this->exit_next_node;
			}
		} else {
			if (this->step_types[0] == STEP_TYPE_ACTION) {
				curr_node = this->actions[0];
			} else if (this->step_types[0] == STEP_TYPE_EXISTING_SCOPE) {
				curr_node = this->existing_scopes[0];
			} else {
				curr_node = this->potential_scopes[0];
			}
		}

		return true;
	} else {
		return false;
	}
}

void SeedExperimentGather::add_to_scope() {
	for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
		if (this->step_types[s_index] == STEP_TYPE_ACTION) {
			this->actions[s_index]->parent = this->scope_context.back();
			this->actions[s_index]->id = this->scope_context.back()->node_counter;
			this->scope_context.back()->node_counter++;
		} else if (this->step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
			this->existing_scopes[s_index]->parent = this->scope_context.back();
			this->existing_scopes[s_index]->id = this->scope_context.back()->node_counter;
			this->scope_context.back()->node_counter++;
		} else {
			this->potential_scopes[s_index]->parent = this->scope_context.back();
			this->potential_scopes[s_index]->id = this->scope_context.back()->node_counter;
			this->scope_context.back()->node_counter++;

			int new_scope_id = solution->scope_counter;
			solution->scope_counter++;
			this->potential_scopes[s_index]->scope->id = new_scope_id;

			for (map<int, AbstractNode*>::iterator it = this->potential_scopes[s_index]->scope->nodes.begin();
					it != this->potential_scopes[s_index]->scope->nodes.end(); it++) {
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

	AbstractNode* end_node;
	if (this->exit_depth > 0) {
		ExitNode* new_exit_node = new ExitNode();
		new_exit_node->parent = this->scope_context.back();
		new_exit_node->id = this->scope_context.back()->node_counter;
		this->scope_context.back()->node_counter++;

		new_exit_node->exit_depth = this->exit_depth;
		new_exit_node->next_node_parent_id = this->scope_context[this->scope_context.size()-1 - exit_depth]->id;
		new_exit_node->next_node = this->exit_next_node;

		this->exit_node = new_exit_node;

		end_node = new_exit_node;
	} else {
		end_node = this->exit_next_node;
	}

	for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
		AbstractNode* next_node;
		if (s_index == (int)this->step_types.size()-1) {
			/**
			 * - end_node_id may not be initialized if from filter branch_node
			 */
			next_node = end_node;
		} else {
			if (this->step_types[s_index+1] == STEP_TYPE_ACTION) {
				next_node = this->actions[s_index+1];
			} else if (this->step_types[s_index+1] == STEP_TYPE_EXISTING_SCOPE) {
				next_node = this->existing_scopes[s_index+1];
			} else {
				next_node = this->potential_scopes[s_index+1];
			}
		}

		if (this->step_types[s_index] == STEP_TYPE_ACTION) {
			this->actions[s_index]->next_node = next_node;
		} else if (this->step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
			this->existing_scopes[s_index]->next_node = next_node;
		} else {
			this->potential_scopes[s_index]->next_node = next_node;
		}
	}

	this->is_candidate = false;
}

void SeedExperimentGather::finalize() {
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
		if (this->step_types[s_index] == STEP_TYPE_ACTION) {
			this->scope_context.back()->nodes[this->actions[s_index]->id] = this->actions[s_index];

			if (this->actions[s_index]->next_node == NULL) {
				this->actions[s_index]->next_node_id = -1;
			} else {
				this->actions[s_index]->next_node_id = this->actions[s_index]->next_node->id;
			}
		} else if (this->step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
			this->scope_context.back()->nodes[this->existing_scopes[s_index]->id] = this->existing_scopes[s_index];

			if (this->existing_scopes[s_index]->next_node == NULL) {
				this->existing_scopes[s_index]->next_node_id = -1;
			} else {
				this->existing_scopes[s_index]->next_node_id = this->existing_scopes[s_index]->next_node->id;
			}
		} else {
			this->scope_context.back()->nodes[this->potential_scopes[s_index]->id] = this->potential_scopes[s_index];

			solution->scopes[this->potential_scopes[s_index]->scope->id] = this->potential_scopes[s_index]->scope;

			if (this->potential_scopes[s_index]->next_node == NULL) {
				this->potential_scopes[s_index]->next_node_id = -1;
			} else {
				this->potential_scopes[s_index]->next_node_id = this->potential_scopes[s_index]->next_node->id;
			}
		}
	}
	this->actions.clear();
	this->existing_scopes.clear();
	this->potential_scopes.clear();

	if (this->exit_node != NULL) {
		this->scope_context.back()->nodes[this->exit_node->id] = this->exit_node;

		if (this->exit_node->next_node == NULL) {
			this->exit_node->next_node_id = -1;
		} else {
			this->exit_node->next_node_id = this->exit_node->next_node->id;
		}
	}
	this->exit_node = NULL;

	// let success_reset() clean
}

void SeedExperimentGather::backprop(double target_val,
									RunHelper& run_helper,
									AbstractExperimentHistory* history) {
	// do nothing
}
