#include "explore_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "init_network.h"
#include "noop_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "score_network.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

ExploreExperiment::ExploreExperiment(Scope* scope_context,
									 AbstractNode* node_context,
									 bool is_branch,
									 AbstractNode* exit_next_node,
									 vector<vector<int>>& dependencies,
									 bool use_signal,
									 SolutionWrapper* wrapper) {
	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;
	this->exit_next_node = exit_next_node;

	this->use_signal = use_signal;

	this->existing_network = NULL;

	this->dependencies = dependencies;
	for (int d_index = 0; d_index < (int)this->dependencies.size(); d_index++) {
		set_dependency_helper(this->scope_context,
							  this->dependencies[d_index],
							  0,
							  this);
	}

	this->state = EXPLORE_EXPERIMENT_STATE_TRAIN_EXISTING;
	this->state_iter = 0;
}

ExploreExperiment::~ExploreExperiment() {
	for (int d_index = 0; d_index < (int)this->dependencies.size(); d_index++) {
		clear_dependency_helper(this->scope_context,
								this->dependencies[d_index],
								0,
								this);
	}

	switch (this->node_context->type) {
	case NODE_TYPE_NOOP:
		{
			NoopNode* noop_node = (NoopNode*)this->node_context;
			noop_node->experiment = NULL;
		}
		break;
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)this->node_context;
			action_node->experiment = NULL;
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)this->node_context;
			scope_node->experiment = NULL;
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)this->node_context;
			if (this->is_branch) {
				branch_node->branch_experiment = NULL;
			} else {
				branch_node->original_experiment = NULL;
			}
		}
		break;
	}

	if (this->existing_network != NULL) {
		delete this->existing_network;
	}
}

bool ExploreExperiment::further_than(ExploreExperiment* other) {
	if (this->state < other->state) {
		return false;
	} else if (this->state > other->state) {
		return true;
	} else {
		if (this->state_iter < other->state_iter) {
			return false;
		} else if (this->state_iter > other->state_iter) {
			return true;
		} else {
			uniform_int_distribution<int> distribution(0, 1);
			if (distribution(generator) == 0) {
				return false;
			} else {
				return true;
			}
		}
	}
}

ExploreExperimentHistory::ExploreExperimentHistory(ExploreExperiment* experiment) {
	this->experiment = experiment;
}

ExploreExperimentState::ExploreExperimentState(ExploreExperiment* experiment) {
	this->experiment = experiment;
}
