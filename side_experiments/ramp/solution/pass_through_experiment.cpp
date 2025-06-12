#include "pass_through_experiment.h"

#include <iostream>
#include <limits>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

PassThroughExperiment::PassThroughExperiment(Scope* scope_context,
											 AbstractNode* node_context,
											 bool is_branch) {
	this->type = EXPERIMENT_TYPE_PASS_THROUGH;

	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;

	vector<AbstractNode*> possible_exits;

	AbstractNode* starting_node;
	switch (this->node_context->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)this->node_context;
			starting_node = action_node->next_node;
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)this->node_context;
			starting_node = scope_node->next_node;
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)this->node_context;
			if (this->is_branch) {
				starting_node = branch_node->branch_next_node;
			} else {
				starting_node = branch_node->original_next_node;
			}
		}
		break;
	case NODE_TYPE_OBS:
		{
			ObsNode* obs_node = (ObsNode*)this->node_context;
			starting_node = obs_node->next_node;
		}
		break;
	}

	this->scope_context->random_exit_activate(
		starting_node,
		possible_exits);

	uniform_int_distribution<int> distribution(0, possible_exits.size()-1);
	int random_index = distribution(generator);
	this->exit_next_node = possible_exits[random_index];

	geometric_distribution<int> geo_distribution(0.2);
	int new_num_steps = geo_distribution(generator);
	if (this->scope_context->exceeded) {
		if (new_num_steps > random_index/2-1) {
			new_num_steps = random_index/2-1;
		}
		if (new_num_steps < 0) {
			new_num_steps = 0;
		}
	}
	if (random_index == 0) {
		new_num_steps++;
	}

	/**
	 * - always give raw actions a large weight
	 *   - existing scopes often learned to avoid certain patterns
	 *     - which can prevent innovation
	 */
	uniform_int_distribution<int> scope_distribution(0, 1);
	for (int s_index = 0; s_index < new_num_steps; s_index++) {
		if (scope_distribution(generator) == 0 && this->scope_context->child_scopes.size() > 0) {
			this->step_types.push_back(STEP_TYPE_SCOPE);
			this->actions.push_back(Action());

			uniform_int_distribution<int> child_scope_distribution(0, this->scope_context->child_scopes.size()-1);
			this->scopes.push_back(this->scope_context->child_scopes[child_scope_distribution(generator)]);
		} else {
			this->step_types.push_back(STEP_TYPE_ACTION);

			this->actions.push_back(problem_type->random_action());

			this->scopes.push_back(NULL);
		}
	}

	this->existing_sum_score = 0.0;
	this->existing_count = 0;
	this->new_sum_score = 0.0;
	this->new_count = 0;

	this->state = PASS_THROUGH_EXPERIMENT_STATE_MEASURE_1_PERCENT;
	this->state_iter = 0;
}

PassThroughExperiment::~PassThroughExperiment() {
	this->node_context->experiment = NULL;
}

void PassThroughExperiment::clean_inputs(Scope* scope,
										 int node_id) {
	if (this->exit_next_node != NULL) {
		if (this->scope_context == scope
				&& this->exit_next_node->id == node_id) {
			delete this;
		}
	}
}

void PassThroughExperiment::clean_inputs(Scope* scope) {
	// do nothing
}

void PassThroughExperiment::replace_factor(Scope* scope,
										   int original_node_id,
										   int original_factor_index,
										   int new_node_id,
										   int new_factor_index) {
	// do nothing
}

void PassThroughExperiment::replace_obs_node(Scope* scope,
											 int original_node_id,
											 int new_node_id) {
	// do nothing
}

void PassThroughExperiment::replace_scope(Scope* original_scope,
										  Scope* new_scope,
										  int new_scope_node_id) {
	for (int a_index = 0; a_index < (int)this->scopes.size(); a_index++) {
		if (this->scopes[a_index] == original_scope) {
			this->scopes[a_index] = new_scope;
		}
	}
}

PassThroughExperimentHistory::PassThroughExperimentHistory(
		PassThroughExperiment* experiment) {
	this->experiment = experiment;

	switch (experiment->state) {
	case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_1_PERCENT:
		{
			uniform_int_distribution<int> active_distribution(0, 99);
			if (active_distribution(generator) == 0) {
				this->is_active = true;
			} else {
				this->is_active = false;
			}
		}
		break;
	case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_5_PERCENT:
		{
			uniform_int_distribution<int> active_distribution(0, 19);
			if (active_distribution(generator) == 0) {
				this->is_active = true;
			} else {
				this->is_active = false;
			}
		}
		break;
	case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_10_PERCENT:
		{
			uniform_int_distribution<int> active_distribution(0, 9);
			if (active_distribution(generator) == 0) {
				this->is_active = true;
			} else {
				this->is_active = false;
			}
		}
		break;
	case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_25_PERCENT:
		{
			uniform_int_distribution<int> active_distribution(0, 3);
			if (active_distribution(generator) == 0) {
				this->is_active = true;
			} else {
				this->is_active = false;
			}
		}
		break;
	case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_50_PERCENT:
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
