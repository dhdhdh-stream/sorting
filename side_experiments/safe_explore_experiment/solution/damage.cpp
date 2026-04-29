#include "damage.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "start_node.h"

using namespace std;

Damage::Damage(AbstractNode* node_context,
			   bool is_branch,
			   SolutionWrapper* wrapper) {
	this->type = EXPERIMENT_TYPE_DAMAGE;

	this->new_scope = NULL;

	vector<AbstractNode*> possible_exits;

	AbstractNode* starting_node;
	switch (node_context->type) {
	case NODE_TYPE_START:
		{
			StartNode* start_node = (StartNode*)node_context;
			starting_node = start_node->next_node;
		}
		break;
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)node_context;
			starting_node = action_node->next_node;
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)node_context;
			starting_node = scope_node->next_node;
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)node_context;
			if (is_branch) {
				starting_node = branch_node->branch_next_node;
			} else {
				starting_node = branch_node->original_next_node;
			}
		}
		break;
	case NODE_TYPE_OBS:
		{
			ObsNode* obs_node = (ObsNode*)node_context;
			starting_node = obs_node->next_node;
		}
		break;
	}

	node_context->parent->random_exit_activate(
		starting_node,
		possible_exits);

	while (true) {
		geometric_distribution<int> exit_distribution(0.1);
		int random_index;
		while (true) {
			random_index = exit_distribution(generator);
			if (random_index < (int)possible_exits.size()) {
				break;
			}
		}
		this->exit_next_node = possible_exits[random_index];

		set<AbstractNode*> children;
		gather_all_children_helper(this->exit_next_node,
								   children);

		bool exit_dangerous = false;
		for (set<AbstractNode*>::iterator it = children.begin();
				it != children.end(); it++) {
			if ((*it)->type == NODE_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)(*it);
				if (action_node->branch_node != NULL) {
					if (children.find(action_node->branch_node) == children.end()) {
						exit_dangerous = true;
					}
				}
			}
		}
		if (!exit_dangerous) {
			break;
		}
	}

	uniform_int_distribution<int> is_dangerous_distribution(0, 1);
	if (is_dangerous_distribution(generator) == 0) {
		this->is_dangerous = true;

		uniform_int_distribution<int> num_steps_distribution(1, 5);
		int new_num_steps = num_steps_distribution(generator);
		for (int s_index = 0; s_index < new_num_steps; s_index++) {
			this->step_types.push_back(STEP_TYPE_ACTION);

			this->actions.push_back(-1);

			this->scopes.push_back(NULL);
		}
	} else {
		this->is_dangerous = false;

		uniform_int_distribution<int> new_scope_distribution(0, 3);
		if (wrapper->solution->state == SOLUTION_STATE_OUTER) {
			this->new_scope = outer_create_new_scope(wrapper);
		} else if (new_scope_distribution(generator) == 0) {
			this->new_scope = create_new_scope(node_context->parent,
											   wrapper);
		}
		if (this->new_scope != NULL) {
			this->step_types.push_back(STEP_TYPE_SCOPE);
			this->actions.push_back(-1);
			this->scopes.push_back(this->new_scope);
		} else {
			bool exit_is_next;
			switch (node_context->type) {
			case NODE_TYPE_START:
				{
					StartNode* start_node = (StartNode*)node_context;
					if (this->exit_next_node == start_node->next_node) {
						exit_is_next = true;
					} else {
						exit_is_next = false;
					}
				}
				break;
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)node_context;
					if (this->exit_next_node == action_node->next_node) {
						exit_is_next = true;
					} else {
						exit_is_next = false;
					}
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)node_context;
					if (this->exit_next_node == scope_node->next_node) {
						exit_is_next = true;
					} else {
						exit_is_next = false;
					}
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)node_context;
					if (is_branch) {
						if (this->exit_next_node == branch_node->branch_next_node) {
							exit_is_next = true;
						} else {
							exit_is_next = false;
						}
					} else {
						if (this->exit_next_node == branch_node->original_next_node) {
							exit_is_next = true;
						} else {
							exit_is_next = false;
						}
					}
				}
				break;
			case NODE_TYPE_OBS:
				{
					ObsNode* obs_node = (ObsNode*)node_context;
					if (this->exit_next_node == obs_node->next_node) {
						exit_is_next = true;
					} else {
						exit_is_next = false;
					}
				}
				break;
			}

			int new_num_steps;
			geometric_distribution<int> geo_distribution(0.3);
			/**
			 * - num_steps less than exit length on average to reduce solution size
			 */
			if (exit_is_next) {
				new_num_steps = 1 + geo_distribution(generator);
			} else {
				new_num_steps = geo_distribution(generator);
			}

			vector<int> possible_child_indexes;
			for (int c_index = 0; c_index < (int)node_context->parent->child_scopes.size(); c_index++) {
				if (node_context->parent->child_scopes[c_index]->nodes.size() > 1) {
					possible_child_indexes.push_back(c_index);
				}
			}
			uniform_int_distribution<int> child_index_distribution(0, possible_child_indexes.size()-1);
			for (int s_index = 0; s_index < new_num_steps; s_index++) {
				bool is_scope = false;
				if (possible_child_indexes.size() > 0) {
					if (possible_child_indexes.size() <= RAW_ACTION_WEIGHT) {
						uniform_int_distribution<int> scope_distribution(0, possible_child_indexes.size() + RAW_ACTION_WEIGHT - 1);
						if (scope_distribution(generator) < (int)possible_child_indexes.size()) {
							is_scope = true;
						}
					} else {
						uniform_int_distribution<int> scope_distribution(0, 1);
						if (scope_distribution(generator) == 0) {
							is_scope = true;
						}
					}
				}
				if (is_scope) {
					this->step_types.push_back(STEP_TYPE_SCOPE);
					this->actions.push_back(-1);

					int child_index = possible_child_indexes[child_index_distribution(generator)];
					this->scopes.push_back(node_context->parent->child_scopes[child_index]);
				} else {
					this->step_types.push_back(STEP_TYPE_ACTION);

					this->actions.push_back(-1);

					this->scopes.push_back(NULL);
				}
			}
		}
	}
}

Damage::~Damage() {
	if (this->new_scope != NULL) {
		delete this->new_scope;
	}
}

void Damage::experiment_check_activate(
		AbstractNode* experiment_node,
		bool is_branch,
		SolutionWrapper* wrapper) {
	DamageState* new_experiment_state = new DamageState(this);
	new_experiment_state->step_index = 0;
	wrapper->experiment_context.back() = new_experiment_state;
}

void Damage::experiment_step(std::vector<double>& obs,
							 int& action,
							 bool& is_next,
							 bool& fetch_action,
							 SolutionWrapper* wrapper) {
	DamageState* experiment_state = (DamageState*)wrapper->experiment_context.back();

	if (experiment_state->step_index == 0) {
		if (this->is_dangerous) {
			bool hit_mine = simulate_helper(wrapper);

			if (hit_mine) {
				delete experiment_state;
				wrapper->experiment_context.back() = NULL;

				return;
			}
		}
	}

	if (experiment_state->step_index >= (int)this->step_types.size()) {
		wrapper->node_context.back() = this->exit_next_node;

		delete experiment_state;
		wrapper->experiment_context.back() = NULL;
	} else {
		if (this->step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			is_next = true;
			fetch_action = true;

			wrapper->num_actions++;
		} else {
			ScopeHistory* inner_scope_history = new ScopeHistory(this->scopes[experiment_state->step_index]);
			wrapper->scope_histories.push_back(inner_scope_history);
			wrapper->node_context.push_back(this->scopes[experiment_state->step_index]->nodes[0]);
			wrapper->experiment_context.push_back(NULL);
		}
	}
}

void Damage::set_action(int action,
						SolutionWrapper* wrapper) {
	DamageState* experiment_state = (DamageState*)wrapper->experiment_context.back();

	if (this->is_dangerous) {
		this->actions[experiment_state->step_index] = action;
	} else {
		uniform_int_distribution<int> action_distribution(0, 6);
		while (true) {
			int potential_action = action_distribution(generator);
			if (potential_action != 4
					&& potential_action != 5) {
				this->actions[experiment_state->step_index] = potential_action;
				break;
			}
		}
	}

	experiment_state->step_index++;
}

void Damage::experiment_exit_step(SolutionWrapper* wrapper) {
	DamageState* experiment_state = (DamageState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];

	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void Damage::result_check_activate(AbstractNode* experiment_node,
								   vector<double>& obs,
								   SolutionWrapper* wrapper) {
	// do nothing
}

void Damage::result_step(vector<double>& obs,
						 int& action,
						 bool& is_next,
						 bool& fetch_action,
						 SolutionWrapper* wrapper) {
	DamageState* experiment_state = (DamageState*)wrapper->result_experiment_context.back();
	if (experiment_state->step_index >= (int)this->step_types.size()) {
		wrapper->result_node_context.back() = this->exit_next_node;

		delete experiment_state;
		wrapper->result_experiment_context.back() = NULL;
	} else {
		if (this->step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			if (this->actions[experiment_state->step_index] == -1) {
				is_next = true;
				fetch_action = true;

				wrapper->result_num_actions++;
			} else {
				action = this->actions[experiment_state->step_index];
				is_next = true;

				wrapper->result_num_actions++;

				experiment_state->step_index++;
			}
		} else {
			ScopeHistory* inner_scope_history = new ScopeHistory(this->scopes[experiment_state->step_index]);
			wrapper->result_scope_histories.push_back(inner_scope_history);
			wrapper->result_node_context.push_back(this->scopes[experiment_state->step_index]->nodes[0]);
			wrapper->result_experiment_context.push_back(NULL);
		}
	}
}

void Damage::result_set_action(int action,
							   SolutionWrapper* wrapper) {
	DamageState* experiment_state = (DamageState*)wrapper->result_experiment_context.back();

	if (this->is_dangerous) {
		this->actions[experiment_state->step_index] = action;
	} else {
		uniform_int_distribution<int> action_distribution(0, 6);
		while (true) {
			int potential_action = action_distribution(generator);
			if (potential_action != 4
					&& potential_action != 5) {
				this->actions[experiment_state->step_index] = potential_action;
				break;
			}
		}
	}

	experiment_state->step_index++;
}

void Damage::result_exit_step(SolutionWrapper* wrapper) {
	DamageState* experiment_state = (DamageState*)wrapper->result_experiment_context.back();

	delete wrapper->result_scope_histories.back();

	wrapper->result_scope_histories.pop_back();
	wrapper->result_node_context.pop_back();
	wrapper->result_experiment_context.pop_back();

	experiment_state->step_index++;
}

DamageState::DamageState(Damage* damage) {
	this->experiment = damage;
}
