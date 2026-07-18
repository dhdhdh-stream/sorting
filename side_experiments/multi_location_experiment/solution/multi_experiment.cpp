#include "multi_experiment.h"

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

MultiExperiment::MultiExperiment(Scope* scope_context,
								 vector<AbstractNode*>& node_contexts,
								 vector<bool>& is_branch,
								 SolutionWrapper* wrapper) {
	this->scope_context = scope_context;
	this->node_contexts = node_contexts;
	this->is_branch = is_branch;

	this->existing_network = NULL;
	this->new_network = NULL;

	this->average_instances_per_hit = 1.0;

	// temp
	this->sum_vals = 0.0;

	this->state = MULTI_EXPERIMENT_STATE_TRAIN_EXISTING;
	this->state_iter = 0;
}

MultiExperiment::~MultiExperiment() {
	for (int c_index = 0; c_index < (int)this->node_contexts.size(); c_index++) {
		switch (this->node_contexts[c_index]->type) {
		case NODE_TYPE_NOOP:
			{
				NoopNode* noop_node = (NoopNode*)this->node_contexts[c_index];
				noop_node->experiment = NULL;
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)this->node_contexts[c_index];
				action_node->experiment = NULL;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)this->node_contexts[c_index];
				scope_node->experiment = NULL;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)this->node_contexts[c_index];
				if (this->is_branch[c_index]) {
					branch_node->branch_experiment = NULL;
				} else {
					branch_node->original_experiment = NULL;
				}
			}
			break;
		}
	}

	if (this->existing_network != NULL) {
		delete this->existing_network;
	}

	if (this->new_network != NULL) {
		delete this->new_network;
	}
}

MultiExperimentHistory::MultiExperimentHistory(MultiExperiment* experiment) {
	this->experiment = experiment;

	this->num_instances = 0;
}

MultiExperimentState::MultiExperimentState(MultiExperiment* experiment) {
	this->experiment = experiment;
}
