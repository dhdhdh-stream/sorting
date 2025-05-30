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
											 bool is_branch,
											 AbstractNode* exit_next_node) {
	this->type = EXPERIMENT_TYPE_PASS_THROUGH;

	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;
	this->exit_next_node = exit_next_node;

	geometric_distribution<int> geo_distribution(0.2);
	int new_num_steps = geo_distribution(generator);
	switch (this->node_context->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)this->node_context;
			if (action_node->next_node == this->exit_next_node) {
				if (new_num_steps == 0) {
					new_num_steps = 1;
				}
			}
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)this->node_context;
			if (scope_node->next_node == this->exit_next_node) {
				if (new_num_steps == 0) {
					new_num_steps = 1;
				}
			}
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)this->node_context;
			if (this->is_branch) {
				if (branch_node->branch_next_node == this->exit_next_node) {
					if (new_num_steps == 0) {
						new_num_steps = 1;
					}
				}
			} else {
				if (branch_node->original_next_node == this->exit_next_node) {
					if (new_num_steps == 0) {
						new_num_steps = 1;
					}
				}
			}
		}
		break;
	case NODE_TYPE_OBS:
		{
			ObsNode* obs_node = (ObsNode*)this->node_context;
			if (obs_node->next_node == this->exit_next_node) {
				if (new_num_steps == 0) {
					new_num_steps = 1;
				}
			}
		}
		break;
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

	this->state = PASS_THROUGH_EXPERIMENT_STATE_INITIAL;

	this->needs_init = true;

	this->explore_iter = 0;

	this->result = EXPERIMENT_RESULT_NA;
}

void PassThroughExperiment::decrement(AbstractNode* experiment_node) {
	delete this;
}

PassThroughExperimentHistory::PassThroughExperimentHistory(
		PassThroughExperiment* experiment) {
	this->experiment = experiment;
}
