#include "confusion.h"

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "new_scope_experiment.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_wrapper.h"

using namespace std;

const int ITERS_BEFORE_DELETE = 100;

Confusion::Confusion(Scope* scope_context,
					 AbstractNode* node_context,
					 bool is_branch,
					 SolutionWrapper* wrapper) {
	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;

	this->iter = 0;

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

	int new_num_steps;
	geometric_distribution<int> geo_distribution(0.2);
	if (random_index == 0) {
		new_num_steps = 1 + geo_distribution(generator);
	} else {
		new_num_steps = geo_distribution(generator);
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
			this->actions.push_back(-1);

			uniform_int_distribution<int> child_scope_distribution(0, this->scope_context->child_scopes.size()-1);
			this->scopes.push_back(this->scope_context->child_scopes[child_scope_distribution(generator)]);
		} else {
			this->step_types.push_back(STEP_TYPE_ACTION);

			uniform_int_distribution<int> action_distribution(0, wrapper->num_possible_actions-1);
			this->actions.push_back(action_distribution(generator));

			this->scopes.push_back(NULL);
		}
	}
}

void Confusion::check_activate(SolutionWrapper* wrapper) {
	wrapper->num_confusion_instances++;

	uniform_int_distribution<int> is_active_distribution(0, 99);
	if (is_active_distribution(generator) == 0) {
		ConfusionState* new_confusion_state = new ConfusionState(this);
		new_confusion_state->step_index = 0;
		wrapper->confusion_context.back() = new_confusion_state;
	}
}

void Confusion::experiment_step(vector<double>& obs,
								int& action,
								bool& is_next,
								SolutionWrapper* wrapper) {
	ConfusionState* confusion_state = (ConfusionState*)wrapper->confusion_context.back();
	if (this->step_types[confusion_state->step_index] == STEP_TYPE_ACTION) {
		action = this->actions[confusion_state->step_index];
		is_next = true;

		confusion_state->step_index++;
		if (confusion_state->step_index >= (int)this->step_types.size()) {
			wrapper->node_context.back() = this->exit_next_node;

			delete confusion_state;
			wrapper->confusion_context.back() = NULL;

			this->iter++;
			if (this->iter >= ITERS_BEFORE_DELETE) {
				this->node_context->confusion = NULL;
				delete this;
			}
		}
	} else {
		ScopeHistory* inner_scope_history = new ScopeHistory(this->scopes[confusion_state->step_index]);
		wrapper->scope_histories.push_back(inner_scope_history);
		wrapper->node_context.push_back(this->scopes[confusion_state->step_index]->nodes[0]);
		wrapper->experiment_context.push_back(NULL);
		wrapper->confusion_context.push_back(NULL);

		if (this->scopes[confusion_state->step_index]->new_scope_experiment != NULL) {
			this->scopes[confusion_state->step_index]->new_scope_experiment->pre_activate(wrapper);
		}
	}
}

void Confusion::experiment_exit_step(SolutionWrapper* wrapper) {
	ConfusionState* confusion_state = (ConfusionState*)wrapper->confusion_context.back();
	if (this->scopes[confusion_state->step_index]->new_scope_experiment != NULL) {
		this->scopes[confusion_state->step_index]->new_scope_experiment->back_activate(wrapper);
	}

	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();
	wrapper->confusion_context.pop_back();

	confusion_state->step_index++;
	if (confusion_state->step_index >= (int)this->step_types.size()) {
		wrapper->node_context.back() = this->exit_next_node;

		delete confusion_state;
		wrapper->confusion_context.back() = NULL;

		this->iter++;
		if (this->iter >= ITERS_BEFORE_DELETE) {
			this->node_context->confusion = NULL;
			delete this;
		}
	}
}

ConfusionState::ConfusionState(Confusion* confusion) {
	this->confusion = confusion;
}
