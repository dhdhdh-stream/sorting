#include "predict_experiment.h"

#include "action_node.h"
#include "branch_node.h"
#include "noop_node.h"
#include "scope_node.h"
#include "score_network.h"

using namespace std;

PredictExperiment::PredictExperiment(Scope* scope_context,
									 AbstractNode* node_context,
									 bool is_branch,
									 AbstractNode* exit_next_node,
									 bool use_signal) {
	this->diversity_index = -1;

	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;
	this->exit_next_node = exit_next_node;

	this->use_signal = use_signal;

	this->existing_network = NULL;
	this->new_network = NULL;

	this->state_iter = 0;
}

PredictExperiment::~PredictExperiment() {
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

	if (this->new_network != NULL) {
		delete this->new_network;
	}
}

PredictExperimentHistory::PredictExperimentHistory(PredictExperiment* experiment) {
	this->experiment = experiment;
}
