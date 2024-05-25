#include "eval_pass_through_experiment.h"

#include <iostream>

#include "action_node.h"
#include "constants.h"
#include "eval.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_scope_node.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

EvalPassThroughExperiment::EvalPassThroughExperiment(
		Eval* eval_context,
		AbstractNode* node_context,
		bool is_branch) {
	this->type = EXPERIMENT_TYPE_EVAL_PASS_THROUGH;

	this->eval_context = eval_context;
	this->scope_context = this->eval_context->subscope;
	this->node_context = node_context;
	this->is_branch = is_branch;

	this->network = NULL;

	this->ending_node = NULL;

	vector<AbstractNode*> possible_exits;

	if (this->node_context->type == NODE_TYPE_ACTION
			&& ((ActionNode*)this->node_context)->next_node == NULL) {
		possible_exits.push_back(NULL);
	}

	AbstractNode* starting_node;
	switch (this->node_context->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)this->node_context;
			starting_node = action_node->next_node;
		}
		break;
	case NODE_TYPE_INFO_SCOPE:
		{
			InfoScopeNode* info_scope_node = (InfoScopeNode*)this->node_context;
			starting_node = info_scope_node->next_node;
		}
		break;
	case NODE_TYPE_INFO_BRANCH:
		{
			InfoBranchNode* info_branch_node = (InfoBranchNode*)this->node_context;
			if (this->is_branch) {
				starting_node = info_branch_node->branch_next_node;
			} else {
				starting_node = info_branch_node->original_next_node;
			}
		}
		break;
	}

	this->scope_context->random_exit_activate(
		starting_node,
		possible_exits);

	uniform_int_distribution<int> distribution(0, possible_exits.size()-1);
	int random_index = distribution(generator);
	this->exit_next_node = possible_exits[random_index];

	this->info_scope = get_existing_info_scope();
	uniform_int_distribution<int> negate_distribution(0, 1);
	this->is_negate = negate_distribution(generator) == 0;

	int new_num_steps;
	uniform_int_distribution<int> uniform_distribution(0, 1);
	geometric_distribution<int> geometric_distribution(0.5);
	if (random_index == 0) {
		new_num_steps = 1 + uniform_distribution(generator) + geometric_distribution(generator);
	} else {
		new_num_steps = uniform_distribution(generator) + geometric_distribution(generator);
	}

	for (int s_index = 0; s_index < new_num_steps; s_index++) {
		InfoScopeNode* new_scope_node = create_existing_info_scope_node();
		if (new_scope_node != NULL) {
			this->step_types.push_back(STEP_TYPE_SCOPE);
			this->actions.push_back(NULL);

			this->scopes.push_back(new_scope_node);
		} else {
			this->step_types.push_back(STEP_TYPE_ACTION);

			ActionNode* new_action_node = new ActionNode();
			new_action_node->action = problem_type->random_action();
			this->actions.push_back(new_action_node);

			this->scopes.push_back(NULL);
		}
	}

	this->new_score = 0.0;

	this->eval_histories.reserve(NUM_DATAPOINTS);
	this->target_val_histories.reserve(NUM_DATAPOINTS);

	this->state = EVAL_PASS_THROUGH_EXPERIMENT_STATE_EXPLORE;
	this->state_iter = 0;

	this->result = EXPERIMENT_RESULT_NA;
}

EvalPassThroughExperiment::~EvalPassThroughExperiment() {
	cout << "outer delete " << this << endl;

	if (this->network != NULL) {
		delete this->network;
	}

	for (int s_index = 0; s_index < (int)this->actions.size(); s_index++) {
		if (this->actions[s_index] != NULL) {
			delete this->actions[s_index];
		}
	}

	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		if (this->scopes[s_index] != NULL) {
			delete this->scopes[s_index];
		}
	}

	if (this->ending_node != NULL) {
		delete this->ending_node;
	}

	for (int h_index = 0; h_index < (int)this->eval_histories.size(); h_index++) {
		delete this->eval_histories[h_index];
	}
}

void EvalPassThroughExperiment::decrement(AbstractNode* experiment_node) {
	delete this;
}

EvalPassThroughExperimentHistory::EvalPassThroughExperimentHistory(
		EvalPassThroughExperiment* experiment) {
	this->experiment = experiment;
}
