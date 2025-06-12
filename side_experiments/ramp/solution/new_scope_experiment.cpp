#include "new_scope_experiment.h"

#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

NewScopeExperiment::NewScopeExperiment(Scope* scope_context,
									   AbstractNode* node_context,
									   bool is_branch) {
	this->type = EXPERIMENT_TYPE_NEW_SCOPE;

	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;

	vector<AbstractNode*> possible_exits;

	AbstractNode* random_start_node;
	switch (node_context->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)node_context;
			random_start_node = action_node->next_node;
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)node_context;
			random_start_node = scope_node->next_node;
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)node_context;
			if (is_branch) {
				random_start_node = branch_node->branch_next_node;
			} else {
				random_start_node = branch_node->original_next_node;
			}
		}
		break;
	case NODE_TYPE_OBS:
		{
			ObsNode* obs_node = (ObsNode*)node_context;
			random_start_node = obs_node->next_node;
		}
		break;
	}

	this->scope_context->random_exit_activate(
		random_start_node,
		possible_exits);

	uniform_int_distribution<int> exit_distribution(0, possible_exits.size()-1);
	this->exit_next_node = possible_exits[exit_distribution(generator)];

	this->successful_scope_node = NULL;

	this->existing_sum_score = 0.0;
	this->existing_count = 0;
	this->new_sum_score = 0.0;
	this->new_count = 0;

	this->scope_context->test_experiments.push_back(this);

	this->state = NEW_SCOPE_EXPERIMENT_STATE_MEASURE_1_PERCENT;
	this->state_iter = 0;
}

NewScopeExperiment::~NewScopeExperiment() {
	for (int e_index = 0; e_index < (int)this->scope_context->test_experiments.size(); e_index++) {
		if (this->scope_context->test_experiments[e_index] == this) {
			this->scope_context->test_experiments.erase(this->scope_context->test_experiments.begin() + e_index);
			break;
		}
	}
	for (int e_index = 0; e_index < (int)this->scope_context->successful_experiments.size(); e_index++) {
		if (this->scope_context->successful_experiments[e_index] == this) {
			this->scope_context->successful_experiments.erase(this->scope_context->successful_experiments.begin() + e_index);
			break;
		}
	}

	this->node_context->experiment = NULL;

	if (this->successful_scope_node != NULL) {
		delete this->successful_scope_node;
	}
}

void NewScopeExperiment::clean_inputs(Scope* scope,
									  int node_id) {
	if (this->exit_next_node != NULL) {
		if (this->scope_context == scope
				&& this->exit_next_node->id == node_id) {
			delete this;
		}
	}
}

void NewScopeExperiment::clean_inputs(Scope* scope) {
	// do nothing
}

void NewScopeExperiment::replace_factor(Scope* scope,
										int original_node_id,
										int original_factor_index,
										int new_node_id,
										int new_factor_index) {
	// do nothing
}

void NewScopeExperiment::replace_obs_node(Scope* scope,
										  int original_node_id,
										  int new_node_id) {
	// do nothing
}

void NewScopeExperiment::replace_scope(Scope* original_scope,
									   Scope* new_scope,
									   int new_scope_node_id) {
	// do nothing
}

NewScopeExperimentHistory::NewScopeExperimentHistory(
		NewScopeExperiment* experiment) {
	this->experiment = experiment;

	switch (experiment->state) {
	case NEW_SCOPE_EXPERIMENT_STATE_MEASURE_1_PERCENT:
		{
			uniform_int_distribution<int> active_distribution(0, 99);
			if (active_distribution(generator) == 0) {
				this->is_active = true;
			} else {
				this->is_active = false;
			}
		}
		break;
	case NEW_SCOPE_EXPERIMENT_STATE_MEASURE_5_PERCENT:
		{
			uniform_int_distribution<int> active_distribution(0, 19);
			if (active_distribution(generator) == 0) {
				this->is_active = true;
			} else {
				this->is_active = false;
			}
		}
		break;
	case NEW_SCOPE_EXPERIMENT_STATE_MEASURE_10_PERCENT:
		{
			uniform_int_distribution<int> active_distribution(0, 9);
			if (active_distribution(generator) == 0) {
				this->is_active = true;
			} else {
				this->is_active = false;
			}
		}
		break;
	case NEW_SCOPE_EXPERIMENT_STATE_MEASURE_25_PERCENT:
		{
			uniform_int_distribution<int> active_distribution(0, 3);
			if (active_distribution(generator) == 0) {
				this->is_active = true;
			} else {
				this->is_active = false;
			}
		}
		break;
	case NEW_SCOPE_EXPERIMENT_STATE_MEASURE_50_PERCENT:
	case NEW_SCOPE_EXPERIMENT_STATE_SUCCESS:
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
