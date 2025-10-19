#include "new_scope_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "start_node.h"

using namespace std;

NewScopeExperiment::NewScopeExperiment(Scope* scope_context,
									   AbstractNode* node_context,
									   bool is_branch,
									   Scope* new_scope,
									   bool is_new) {
	this->type = EXPERIMENT_TYPE_NEW_SCOPE;

	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;

	this->new_scope = new_scope;
	this->is_new = is_new;

	this->node_context->experiment = this;

	vector<AbstractNode*> possible_exits;

	AbstractNode* starting_node;
	switch (this->node_context->type) {
	case NODE_TYPE_START:
		{
			StartNode* start_node = (StartNode*)this->node_context;
			starting_node = start_node->next_node;
		}
		break;
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

	geometric_distribution<int> exit_distribution(0.1);
	int random_index;
	while (true) {
		random_index = exit_distribution(generator);
		if (random_index < (int)possible_exits.size()) {
			break;
		}
	}
	this->exit_next_node = possible_exits[random_index];

	this->new_sum_scores = 0.0;

	this->state = NEW_SCOPE_EXPERIMENT_STATE_C1;
	this->state_iter = 0;

	this->result = EXPERIMENT_RESULT_NA;
}

NewScopeExperiment::~NewScopeExperiment() {
	if (this->new_scope != NULL
			&& this->is_new) {
		delete this->new_scope;
	}

	for (int h_index = 0; h_index < (int)this->new_scope_histories.size(); h_index++) {
		delete this->new_scope_histories[h_index];
	}
}

NewScopeExperimentHistory::NewScopeExperimentHistory(NewScopeExperiment* experiment) {
	this->experiment = experiment;

	this->is_hit = false;
}

NewScopeExperimentState::NewScopeExperimentState(NewScopeExperiment* experiment) {
	this->experiment = experiment;
}
