#include "explore_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "network.h"
#include "noop_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

ExploreExperiment::ExploreExperiment(SolutionWrapper* wrapper,
									 int experiment_index,
									 Scope* scope_context,
									 AbstractNode* node_context,
									 bool is_branch,
									 AbstractNode* exit_next_node) {
	this->wrapper = wrapper;
	this->experiment_index = experiment_index;

	this->create_timestamp = wrapper->solution->timestamp;

	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;
	this->exit_next_node = exit_next_node;

	this->existing_network = NULL;
	this->new_network = NULL;

	this->try_iter = 0;

	this->sum_vals = 0.0;

	this->state = EXPLORE_EXPERIMENT_STATE_TRAIN_EXISTING;
	this->state_iter = 0;
}

ExploreExperiment::~ExploreExperiment() {
	this->wrapper->experiments[this->experiment_index] = NULL;

	switch (this->node_context->type) {
	case NODE_TYPE_NOOP:
		{
			NoopNode* noop_node = (NoopNode*)this->node_context;
			for (int e_index = 0; e_index < (int)noop_node->experiments.size(); e_index++) {
				if (noop_node->experiments[e_index] == this) {
					noop_node->experiments.erase(noop_node->experiments.begin() + e_index);
					break;
				}
			}
		}
		break;
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)this->node_context;
			for (int e_index = 0; e_index < (int)action_node->experiments.size(); e_index++) {
				if (action_node->experiments[e_index] == this) {
					action_node->experiments.erase(action_node->experiments.begin() + e_index);
					break;
				}
			}
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)this->node_context;
			for (int e_index = 0; e_index < (int)scope_node->experiments.size(); e_index++) {
				if (scope_node->experiments[e_index] == this) {
					scope_node->experiments.erase(scope_node->experiments.begin() + e_index);
					break;
				}
			}
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)this->node_context;
			if (this->is_branch) {
				for (int e_index = 0; e_index < (int)branch_node->branch_experiments.size(); e_index++) {
					if (branch_node->branch_experiments[e_index] == this) {
						branch_node->branch_experiments.erase(branch_node->branch_experiments.begin() + e_index);
						break;
					}
				}
			} else {
				for (int e_index = 0; e_index < (int)branch_node->original_experiments.size(); e_index++) {
					if (branch_node->original_experiments[e_index] == this) {
						branch_node->original_experiments.erase(branch_node->original_experiments.begin() + e_index);
						break;
					}
				}
			}
		}
		break;
	}

	if (this->existing_network != NULL) {
		delete this->existing_network;
	}

	if (this->new_network != NULL) {
		delete this->new_network;
	}
}

bool ExploreExperiment::is_existing() {
	if (this->state == EXPLORE_EXPERIMENT_STATE_TRAIN_EXISTING) {
		return true;
	} else {
		return false;
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
